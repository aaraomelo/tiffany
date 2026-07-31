/* carnot.c — η DERIVADO DE ∮, e o veredito sobre a minha conjectura da cónica.
 *
 * Ficou em aberto no termica.c: eu anotei (1−e)/(1+e) como CANDIDATO a T_f/T_q, e disse que o
 * que testaria isso seria derivar η de ∮ e não de semelhança de forma. Aqui deriva-se.
 *
 * A derivação é curta e é exata:
 *
 *   ciclo fecha           ∮dU = 0   ⟹   W = Q_q − Q_f
 *   reversível            ∮dQ/T = 0 ⟹   Q_q/T_q = Q_f/T_f   ⟹   Q_f/Q_q = T_f/T_q
 *   donde                 η = W/Q_q = 1 − Q_f/Q_q = 1 − T_f/T_q
 *
 * É o ∮ que faz o trabalho: a primeira integral dá W, a segunda converte a razão dos calores na
 * razão das temperaturas. Sem a segunda, η fica em Q e não em T.
 *
 *   §C1  o ciclo fecha: ∮dU = 0, logo W = Q_q − Q_f — exato em ℚ
 *   §C2  reversível: ∮dQ/T = 0, e daí Q_f/Q_q = T_f/T_q — exato em ℚ
 *   §C3  η = 1 − T_f/T_q, derivado, e igual a W/Q_q em toda a varredura
 *   §C4  IRREVERSÍVEL: ∮dQ/T < 0, e então η CAI abaixo de Carnot — medido
 *   §C5  o veredito sobre a minha conjectura: era parametrização, NÃO teorema
 *
 *   cc -O2 -std=c99 carnot.c -o carnot && ./carnot
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static Par q(long n, long d){ return ra_classe((Par){n,d}); }
static Par qsub(Par x, Par y){ return ra_soma(x, (Par){-y.a, y.b}); }
static Par qinv(Par x){ Par r = { x.b, x.a }; return ra_classe(r); }
static int  qeq(Par x, Par y){ x=ra_classe(x); y=ra_classe(y); return x.a==y.a && x.b==y.b; }

int main(void){
printf("\n=== η DERIVADO DE ∮ =======================================================\n");
printf("    Duas integrais: a primeira dá W, a segunda troca calores por temperaturas.\n");

printf("\n§C1  O ciclo FECHA: ∮dU = 0, logo W = Q_q − Q_f.\n\n");
{
    int mau = 0; long casos = 0;
    for(long qn_ = 1; qn_ <= 20; qn_++) for(long qd = 1; qd <= 8; qd++)
    for(long fn = 1; fn <= 20; fn++) for(long fd = 1; fd <= 8; fd++){
        Par Qq = q(qn_,qd), Qf = q(fn,fd);
        Par W = qsub(Qq, Qf);
        /* ∮dU = Q_q − Q_f − W tem de ser ZERO: o fluido volta ao mesmo estado */
        if(!qeq(qsub(qsub(Qq,Qf), W), q(0,1))) mau++;
        casos++;
    }
    ok("∮dU = 0 dá W = Q_q − Q_f, exato — é a primeira lei sobre o ciclo", mau == 0);
    printf("      (%ld ciclos.)\n", casos);
}

printf("\n§C2  REVERSÍVEL: ∮dQ/T = 0, e daí Q_f/Q_q = T_f/T_q.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      T_q     T_f     Q_q     Q_f = Q_q·T_f/T_q   ∮dQ/T\n");
    for(long tq = 2; tq <= 20; tq++) for(long tf = 1; tf < tq; tf++)
    for(long qn_ = 1; qn_ <= 12; qn_++){
        Par Tq = q(tq,1), Tf = q(tf,1), Qq = q(qn_,1);
        /* o reversível define Q_f pela igualdade das entropias trocadas */
        Par Qf = ra_prod(Qq, ra_prod(Tf, qinv(Tq)));
        Par ciclo = qsub(ra_prod(Qq, qinv(Tq)), ra_prod(Qf, qinv(Tf)));
        if(!qeq(ciclo, q(0,1))) mau++;
        /* e daí a razão dos calores É a razão das temperaturas */
        if(!qeq(ra_prod(Qf, qinv(Qq)), ra_prod(Tf, qinv(Tq)))) mau++;
        if(tq == 4 && tf == 3 && qn_ == 8)
            printf("      %ld/%-5ld %ld/%-5ld %ld/%-5ld %ld/%-18ld %ld/%ld\n",
                   Tq.a,Tq.b, Tf.a,Tf.b, Qq.a,Qq.b, Qf.a,Qf.b, ciclo.a,ciclo.b);
        casos++;
    }
    ok("∮dQ/T = 0 exato, e dele sai Q_f/Q_q = T_f/T_q — a segunda lei sobre o ciclo", mau == 0);
    printf("      (%ld ciclos reversíveis.)\n", casos);
    printf("\n      É AQUI que a temperatura entra. Sem esta integral, η ficaria em calores e nunca\n");
    printf("      chegaria a temperaturas — a razão Q_f/Q_q seria um dado da máquina, não uma lei.\n");
}

printf("\n§C3  η = 1 − T_f/T_q, derivado, e igual a W/Q_q.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      T_q     T_f     η = 1 − T_f/T_q   W/Q_q     iguais?\n");
    for(long tq = 2; tq <= 24; tq++) for(long tf = 1; tf < tq; tf++)
    for(long qn_ = 1; qn_ <= 10; qn_++){
        Par Tq = q(tq,1), Tf = q(tf,1), Qq = q(qn_,1);
        Par Qf = ra_prod(Qq, ra_prod(Tf, qinv(Tq)));
        Par W  = qsub(Qq, Qf);
        Par eta_T = qsub(q(1,1), ra_prod(Tf, qinv(Tq)));
        Par eta_W = ra_prod(W, qinv(Qq));
        if(!qeq(eta_T, eta_W)) mau++;
        /* e 0 < η < 1 sempre que 0 < T_f < T_q */
        if(ra_cmp(eta_T, q(0,1)) <= 0 || ra_cmp(eta_T, q(1,1)) >= 0) mau++;
        if(qn_ == 1 && ((tq==2&&tf==1) || (tq==4&&tf==3) || (tq==20&&tf==1)))
            printf("      %-7ld %-7ld %ld/%-15ld %ld/%-7ld sim ✓\n",
                   tq, tf, eta_T.a, eta_T.b, eta_W.a, eta_W.b);
        casos++;
    }
    ok("η sai de ∮ e coincide com W/Q_q em toda a varredura, exato em ℚ", mau == 0);
    printf("      (%ld ciclos.)\n", casos);
    printf("\n      Derivado, não postulado: a primeira integral deu W, a segunda trocou os calores\n");
    printf("      pelas temperaturas, e η caiu de lá. E fica estritamente entre 0 e 1 — nunca 1,\n");
    printf("      que precisaria de T_f = 0.\n");
}

printf("\n§C4  IRREVERSÍVEL: ∮dQ/T < 0, e η CAI abaixo de Carnot.\n\n");
{
    int mau = 0; long casos = 0, abaixo = 0;
    printf("      T_q  T_f  Q_f extra   ∮dQ/T      η real      η Carnot   caiu?\n");
    for(long tq = 4; tq <= 20; tq++) for(long tf = 1; tf < tq; tf++)
    for(long ex = 1; ex <= 6; ex++){
        Par Tq = q(tq,1), Tf = q(tf,1), Qq = q(10,1);
        Par Qf_rev = ra_prod(Qq, ra_prod(Tf, qinv(Tq)));
        Par Qf = ra_soma(Qf_rev, q(ex,10));          /* rejeita-se MAIS calor: irreversível */
        Par ciclo = qsub(ra_prod(Qq, qinv(Tq)), ra_prod(Qf, qinv(Tf)));
        Par eta = ra_prod(qsub(Qq,Qf), qinv(Qq));
        Par eta_c = qsub(q(1,1), ra_prod(Tf, qinv(Tq)));
        if(ra_cmp(ciclo, q(0,1)) >= 0) mau++;        /* ∮dQ/T < 0 estrito */
        if(ra_cmp(eta, eta_c) >= 0) mau++;           /* e η < η_Carnot */
        abaixo++;
        if(tq==4 && tf==3 && ex==2)
            printf("      %-4ld %-4ld %ld/%-9ld %ld/%-9ld %ld/%-10ld %ld/%-9ld sim ✓\n",
                   tq, tf, q(ex,10).a, q(ex,10).b, ciclo.a, ciclo.b,
                   eta.a, eta.b, eta_c.a, eta_c.b);
        casos++;
    }
    ok("rejeitar calor a mais abre o ∮ e derruba o η — abaixo de Carnot, sempre", mau == 0);
    printf("      (%ld ciclos irreversíveis, %ld abaixo de Carnot.)\n", casos, abaixo);
    printf("\n      O teto de Carnot não é um limite de engenharia: é o ∮ fechado. Abri-lo custa\n");
    printf("      rendimento, e o custo é exatamente o que se abriu.\n");
}

printf("\n§C5  O VEREDITO sobre a minha conjectura da cónica.\n\n");
{
    int mau = 0;
    /* eu tinha anotado (1−e)/(1+e) como candidato a T_f/T_q. Testando: isso É consistente,
     * mas por CONSTRUÇÃO — é uma reparametrização, e qualquer bijeção de (0,1) serviria. */
    printf("      e       (1−e)/(1+e)   se T_f/T_q for isso, η =\n");
    for(long n = 1; n <= 9; n += 4){
        Par e = q(n,10);
        Par razao = ra_prod(qsub(q(1,1),e), qinv(ra_soma(q(1,1),e)));
        Par eta = qsub(q(1,1), razao);
        printf("      %ld/10    %ld/%-11ld %ld/%ld\n", n, razao.a, razao.b, eta.a, eta.b);
        /* consistente: 0 < razão < 1 para 0 < e < 1 — logo dá um η válido */
        if(ra_cmp(razao, q(0,1)) <= 0 || ra_cmp(razao, q(1,1)) >= 0) mau++;
    }
    ok("a razão focal dá um T_f/T_q VÁLIDO — mas isso não a torna A temperatura", mau == 0);
    printf("\n      E aqui está o veredito, que é negativo e é o que interessa: qualquer bijeção de\n");
    printf("      (0,1) em (0,1) daria um T_f/T_q válido. A razão focal ser uma delas não a\n");
    printf("      distingue de nenhuma outra. Para ser TEOREMA faltaria o que eu próprio escrevi\n");
    printf("      que faltava: derivar a temperatura do ∮ SOBRE A CÓNICA — isto é, exibir o ciclo\n");
    printf("      termodinâmico cuja trajetória é a elipse e cujas isotérmicas são os focos.\n");
    printf("\n      Não exibi, e não é por falta de tentar aqui: é que a elipse do CORTE e a elipse\n");
    printf("      de um ciclo no plano P–V não são a mesma curva, e eu não tenho ponte entre elas.\n");
    printf("      Então: PARAMETRIZAÇÃO, não teorema. Fica fechado como negativo.\n");
    printf("\n      O que SOBREVIVE do termica.c é o que já estava medido e não depende disto: o\n");
    printf("      ciclo fecha sse Δ<0, os dois focos são dois, e a janela é aberta nos extremos.\n");
    printf("      Isso é geometria da secção, e vale. A temperatura é que não entrou.\n");
}

printf("\n=== η =====================================================================\n");
printf("  Derivado, e são duas integrais:\n\n");
printf("    ∮dU = 0     ⟹  W = Q_q − Q_f                     (o ciclo volta ao estado)\n");
printf("    ∮dQ/T = 0   ⟹  Q_f/Q_q = T_f/T_q                 (o reversível)\n");
printf("    donde          η = 1 − T_f/T_q, e igual a W/Q_q  — exato em ℚ\n\n");
printf("  E abrir o ∮ (rejeitar calor a mais) derruba o η abaixo de Carnot, sempre: o teto não é\n");
printf("  limite de engenharia, é o ∮ fechado.\n\n");
printf("  VEREDITO NEGATIVO sobre a minha conjectura: a razão focal (1−e)/(1+e) dá um T_f/T_q\n");
printf("  válido, mas qualquer bijeção de (0,1) daria. Sem exibir o ciclo cuja trajetória É a\n");
printf("  elipse do corte, é PARAMETRIZAÇÃO e não teorema. Fecha como negativo.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
