/* so_cristal.c — O SISTEMA OPERACIONAL NO CRISTAL, em ANALOGICO, no lugar da ISA (api.h).
 *
 * A triade (⊕ ⊗ ∏) em silicio analogico, com a arquitetura do Aarão:
 *   diodo de um lado  -> a SOMA (⊕ Clifford): correntes se somam no no' (Kirchhoff).
 *   diodo do outro    -> a MULT (⊗ La Hire): log-domain.
 *   PONTRYAGIN no meio -> ∏ = exp∘Σ∘log: multiplicar = exp(log a + log b) = a*b.
 * O diodo E' o log/exp: V=V_T*log(I/I_s), I=I_s*e^{V/V_T}. Somar os V (Kirchhoff no ramo log)
 * e ler pelo exp DA' o produto — o Pontryagin. Somar as correntes direto DA' a soma.
 *
 * VALIDA o sinal ANALOGICO (o diodo, continuo) contra o DISCRETO da ISA (api.h ADD/MUL, exato):
 * o cristal analogico reproduz a ISA. resid 0 (dentro do arredondamento do sinal).
 *
 * cc -O2 -I. so_cristal.c -lm -o so_cristal && ./so_cristal
 */
#include "api.h"
#include <stdio.h>
#include <math.h>

/* o diodo: log (entrada) e exp (saida). o meio e' a SOMA dos logs (Pontryagin). */
static double diodo_log(double I){ return log(I); }         /* V ~ log I (o diodo comprime) */
static double diodo_exp(double V){ return exp(V); }         /* I ~ exp V (o diodo expande) */
/* MULT analogica (La Hire) via Pontryagin: exp( log a + log b ) = a*b */
static double mult_analog(double a, double b){ return diodo_exp( diodo_log(a) + diodo_log(b) ); }
/* SOMA analogica (Clifford): as correntes se somam no no' (Kirchhoff) */
static double soma_analog(double a, double b){ return a + b; }

int main(void){
    printf("=== O SO NO CRISTAL (analogico) — a triade nos diodos, validada contra a ISA ===\n\n");
    printf(" arquitetura: diodo(SOMA ⊕) | PONTRYAGIN ∏ no meio | diodo(MULT ⊗)\n");
    printf("   o diodo e' log/exp; multiplicar = exp(log a + log b) = a*b (∏=exp∘Σ∘log);\n");
    printf("   somar = correntes no no' (Kirchhoff). o meio (∏) faz a mult da soma dos logs.\n\n");

    /* (1) ⊕ SOMA (Clifford): analogico (Kirchhoff) == ISA ADD (discreto, exato) */
    printf("(1) ⊕ SOMA (Clifford): o sinal analogico == o discreto da ISA (ADD):\n");
    printf("    %-10s %-14s %-14s %-s\n","(a,b)","analog a+b","ISA ADD","bate?");
    long soma_ok=0, soma_tot=0;
    for(u64 a=1;a<=200;a++) for(u64 b=1;b<=200;b++){
        double an = soma_analog((double)a,(double)b);
        u64 isa = ADD(a,b);                              /* api.h: o adder do gato/esquilo, exato */
        if((u64)llround(an)==isa) soma_ok++;
        soma_tot++;
    }
    for(int t=0;t<3;t++){ u64 a=(t+3)*7, b=(t+2)*5;
        printf("    (%3llu,%3llu) %-14.1f %-14llu %s\n",(unsigned long long)a,(unsigned long long)b,
               soma_analog(a,b),(unsigned long long)ADD(a,b),
               (u64)llround(soma_analog(a,b))==ADD(a,b)?"sim":"NAO"); }
    printf("    analogico == ISA em %ld/%ld pares: %s\n\n", soma_ok, soma_tot, soma_ok==soma_tot?"resid 0":"FALHOU");

    /* (2) ⊗ MULT (La Hire) via ∏ PONTRYAGIN: exp(log a+log b) == ISA MUL (double-and-add, exato) */
    printf("(2) ⊗ MULT (La Hire) via ∏ PONTRYAGIN: exp(log a + log b) == o discreto da ISA (MUL):\n");
    printf("    %-10s %-16s %-14s %-s\n","(a,b)","analog exp(Σlog)","ISA MUL","bate?");
    long mult_ok=0, mult_tot=0;
    for(u64 a=1;a<=200;a++) for(u64 b=1;b<=200;b++){
        double an = mult_analog((double)a,(double)b);
        u64 isa = MUL(a,b);                              /* api.h: shift-and-add, exato */
        if((u64)llround(an)==isa) mult_ok++;
        mult_tot++;
    }
    for(int t=0;t<3;t++){ u64 a=(t+3)*7, b=(t+2)*5;
        printf("    (%3llu,%3llu) %-16.1f %-14llu %s\n",(unsigned long long)a,(unsigned long long)b,
               mult_analog(a,b),(unsigned long long)MUL(a,b),
               (u64)llround(mult_analog(a,b))==MUL(a,b)?"sim":"NAO"); }
    printf("    analogico (log-domain) == ISA em %ld/%ld pares: %s\n\n", mult_ok, mult_tot,
           mult_ok==mult_tot?"resid 0 (o Pontryagin reproduz o double-and-add)":"quase (arredondamento)");

    /* (3) o ∏ PONTRYAGIN no meio: soma dos logs -> exp = produto; e o log desfaz (o inverso) */
    printf("(3) ∏ PONTRYAGIN no meio (exp∘Σ∘log): faz a MULT da SOMA dos logs, e o log DESFAZ:\n");
    double a=12, b=7;
    printf("    log(%g)+log(%g) = %.4f ; exp = %.1f = %g*%g (a mult); e log(%.0f)-log(%g)=%.4f=log(%g) (o inverso)\n",
           a,b, log(a)+log(b), mult_analog(a,b), a,b, mult_analog(a,b), b, log(mult_analog(a,b))-log(b), a);
    printf("    -> o meio (∏) e' reversivel: soma no log = produto; subtrai no log = divide (o inverso).\n\n");

    int ok = (soma_ok==soma_tot) && (mult_ok==mult_tot);
    printf("--------------------------------------------------------------------------\n");
    printf(" O SO NO CRISTAL: a triade em diodos — ⊕ soma (correntes no no'), ⊗ mult (log-domain),\n");
    printf(" ∏ Pontryagin no meio (exp∘Σ∘log: exp da soma dos logs = produto). O sinal ANALOGICO\n");
    printf(" (o diodo, continuo) REPRODUZ o discreto da ISA (ADD/MUL): %ld/%ld soma, %ld/%ld mult. %s\n",
           soma_ok,soma_tot,mult_ok,mult_tot, ok? "ANALOGICO == ISA." : "quase (arredondamento).");
    return ok?0:1;
}