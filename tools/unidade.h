/* unidade.h — cada asserção vira um TESTE UNITÁRIO endereçável.
 *
 * Os medidores já eram unitários por dentro: cada um chama ok("o que se afirma", condição), e
 * são 214 dessas chamadas espalhadas por 29 arquivos. O que faltava não era o teste — era o
 * ENDEREÇO. Sem ele a bateria só sabia dizer "o medidor passou" ou "o medidor falhou", e um
 * programa que faz vinte afirmações é grosso demais para servir de unidade.
 *
 * Aqui o ok() passa a emitir DUAS coisas: a linha em português, para quem lê, e uma linha
 * `#UNIT ok|falha  <afirmação>`, para quem conta. A mesma chamada, dois leitores.
 *
 * E é isto que liga a bateria à transformada. A MEMBRANA passa a ser o vetor das 214
 * asserções — uma entrada por teste, 0 quando passa. Por transformada.c §U4,
 *
 *     x = 0  ⟺  ‖Fx‖² = 0
 *
 * então o selo é a norma desse vetor: zero exatamente quando tudo passa, e imune a
 * cancelamento por ser soma de quadrados. E por §U5 o Dirac LOCALIZA — o teste que falhou é
 * o ponto para onde a volta concentra. Antes o selo apontava um programa; agora aponta uma
 * afirmação.
 *
 * Nada de torradeira: quem não mudou de semente não roda. A finura serve para saber ONDE
 * olhar quando alguma coisa se mexe, não para rodar mais.
 */
#ifndef UNIDADE_H
#define UNIDADE_H
#include <stdio.h>

static int falhas = 0;
static int unidades = 0;

static void ok(const char *r, int c){
    unidades++;
    printf("      %-58s %s\n", r, c ? "sim ✓" : "NÃO ✗");
    printf("#UNIT %s  %s\n", c ? "ok   " : "falha", r);
    if(!c) falhas++;
}
#endif
