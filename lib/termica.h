/* termica.h — A RADIAÇÃO NEGRA: as constantes e as leis, para o headjack e o que vier depois.
 *
 * O Aarão: "falta o dual pra fechar o circuito, é a radiação térmica — projetar o array de
 * sensores pra detectar. É a radiação negra, o dual do eletromagnético."
 *
 * Aqui só as leis e as constantes, medidas no `radiacao.c`. Elas são de tabela — CODATA e as
 * definições do SI — e não há uma escolhida por mim.
 */
#ifndef TERMICA_H
#define TERMICA_H
#include <math.h>

#define SIGMA_SB   5.670374419e-8      /* Stefan–Boltzmann, W/(m²·K⁴) — exata no SI de 2019 */
#define WIEN_B     2.897771955e-3      /* deslocamento de Wien, m·K */
#define K_B        1.380649e-23        /* Boltzmann, J/K — exata por definição */
#define H_PLANCK   6.62607015e-34      /* Planck, J·s — exata por definição */
#define C_LUZ      2.99792458e8        /* velocidade da luz, m/s — exata por definição */

/* Stefan–Boltzmann: a potência radiada por unidade de área de um corpo negro */
static double sb_potencia(double T){ return SIGMA_SB * T*T*T*T; }

/* Wien: o comprimento de onda do pico */
static double wien_pico(double T){ return WIEN_B / T; }

/* Planck: a radiância espectral, W/(m²·sr·m) */
static double planck(double lambda, double T){
    double a = 2.0 * H_PLANCK * C_LUZ*C_LUZ / (lambda*lambda*lambda*lambda*lambda);
    double b = H_PLANCK * C_LUZ / (lambda * K_B * T);
    return a / (exp(b) - 1.0);
}

/* a derivada da potência com a temperatura: dP/dT = 4σT³ — é o que um sensor térmico mede */
static double sb_derivada(double T){ return 4.0 * SIGMA_SB * T*T*T; }

#endif
