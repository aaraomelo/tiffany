/* unidade.h — cada asserção vira um TESTE UNITÁRIO endereçável.
 *
 * Os medidores já eram unitários por dentro. O que faltava não era o teste — era o ENDEREÇO:
 * a bateria só sabia dizer "o medidor passou", e um programa que faz vinte afirmações é grosso
 * demais para servir de unidade.
 *
 * Há dois idiomas no repositório, e este arquivo cobre os dois sem reescrever a lógica de
 * nenhum:
 *
 *   ok("o que se afirma", condição)      — 29 medidores. O endereço é a própria frase.
 *   VD(condição, "msg")                   — os outros, que já escreviam
 *                                             printf(… , cond ? "FALHA" : "a mensagem")
 *                                           e agora escrevem printf(… , VD(cond, "a mensagem")).
 *                                           A string devolvida é a MESMA, a saída humana não muda,
 *                                           e o rótulo da unidade é a própria mensagem — que já
 *                                           dizia em português o que se afirmava.
 *
 * As linhas #UNIT saem TODAS no fim, por atexit, e não no meio do texto: quem lê continua a ler
 * o relatório, e quem conta lê o rodapé. Um só lugar, dois leitores.
 *
 * E é isto que liga a bateria à transformada. A MEMBRANA é o vetor das asserções — uma entrada
 * por teste, 0 quando passa. Por transformada.c §U4, x = 0 ⟺ ‖Fx‖² = 0: o selo é a norma, zero
 * exatamente quando tudo passa, e imune a cancelamento por ser soma de quadrados. Por §U5 o
 * Dirac LOCALIZA — antes o selo apontava um programa, agora aponta uma afirmação.
 *
 * A finura serve para saber ONDE olhar quando algo se mexe. Não para rodar mais.
 */
#ifndef UNIDADE_H
#define UNIDADE_H
#include <stdio.h>
#include <stdlib.h>

static int falhas = 0;
static int unidades = 0;

#define UNI_MAX 1024
static const char *uni_rot[UNI_MAX];
static int uni_est[UNI_MAX];
static int uni_n = 0, uni_perdidas = 0, uni_registado = 0;

static void uni_rodape(void){
    for(int i = 0; i < uni_n; i++)
        printf("#UNIT %s  %s\n", uni_est[i] ? "ok   " : "falha", uni_rot[i]);
    if(uni_perdidas)
        printf("#UNIT falha  MAIS DE %d ASSERCOES: %d nao couberam no rodape\n",
               UNI_MAX, uni_perdidas);
}
static void uni_poe(const char *rot, int passou){
    unidades++;
    if(!passou) falhas++;
    if(!uni_registado){ atexit(uni_rodape); uni_registado = 1; }
    if(uni_n < UNI_MAX){ uni_rot[uni_n] = rot; uni_est[uni_n] = passou; uni_n++; }
    else uni_perdidas++;              /* o teto é dito, nunca calado */
}

/* o idioma dos 29: a afirmação em português É o endereço */
static void ok(const char *r, int c){
    printf("      %-58s %s\n", r, c ? "sim ✓" : "NÃO ✗");
    uni_poe(r, c);
}

/* o idioma dos outros: devolve a MESMA string de antes, e regista pelo caminho.
 *
 * A troca é textual e reversível — `cond ? "FALHA" : "a msg"` vira `VD(cond, "a msg")`. E o
 * rótulo da unidade é a PRÓPRIA mensagem de sucesso, que já dizia em português o que estava a
 * ser afirmado: não foi preciso inventar nome para teste nenhum, eles já tinham. */
static const char *uni_vd(int falhou, const char *bom){
    uni_poe(bom, !falhou);
    return falhou ? "FALHA" : bom;
}
#define VD(cond, bom) uni_vd((cond), (bom))


/* CONCLUI — o resumo de uma secção, e NÃO uma unidade.
 *
 * Estas linhas eram `ok("...", 1)`: a constante disfarçada, e havia 66 delas no repositório.
 * Mas o defeito não era afirmarem falso — era serem `ok()`. Elas RESUMEM o que as asserções
 * acima mediram, e um resumo não é uma medida: contadas como unidade, inflacionavam a bateria
 * com frases. Marcadas com [~], a contagem passa a ser só do que mede.
 */
static void conclui(const char *q){ printf("  [~] %s\n", q); }

#endif
