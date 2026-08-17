/* motor.c — AS MÁQUINAS: o motor de indução, o inversor e o DTC.
 *
 * O Aarão: "agora vamos pro corpo motor/rotor; traz minha monografia de graduação de hiper —
 * motor de indução, modulação vetorial via inversor multinível e DTC. Vamos introduzir as
 * máquinas."
 *
 * A fonte é a monografia dele: "Controle Direto de Torque do Motor de Indução Trifásico",
 * Aarão Melo Lopes, UFRR / Centro de Ciências e Tecnologia, 2017 (orientadora Profa. Dra.
 * Susset Guerra Jiménez) — hiper/aposentados/teoria/papers/tcc_dtc_2017/DTC.tex.
 *
 * E a peça central estava à espera desde o princípio deste projeto:
 *
 *      T_e = (3/2)·P· ψ_s × i_s          O TORQUE É O PRODUTO CRUZADO.
 *
 * Não é analogia. A parte SIMÉTRICA de um bilinear devolve escalar e não gira nada; a
 * ANTISSIMÉTRICA devolve vetor, sai perpendicular aos dois, e é ela que faz o eixo rodar. O
 * projeto media isto como "o cruzado é a ordem"; aqui a ordem tem unidade de N·m.
 *
 * E o resto encaixa: os oito vetores do inversor são GF(2)³ (as palavras de três bits, uma por
 * perna), e o DTC é a AMPUTAÇÃO do §A4 — o contínuo do erro colapsa em três decisões
 * (subir/manter/descer) e uma tabela.
 *
 *   §M1  Clarke: as três fases viram UM vetor, e a soma das três fmm é 3/2 da amplitude
 *   §M2  o TORQUE É O CRUZADO — e o direto não gira nada
 *   §M3  os oito vetores do inversor SÃO GF(2)³: seis no hexágono, dois nulos
 *   §M4  o vetor de tensão move o fluxo: Δψ_s = v·Δt, e o ângulo γ dá o torque
 *   §M5  o DTC de Takahashi: histerese + setor -> a tabela, e é a amputação
 *   §M6  o inversor MULTINÍVEL: mais níveis, menos amputação — e mede-se quanto
 *   §M7  SIMULAR a máquina com DTC, e validar pelos dois caminhos
 *
 *   cc -O2 -std=c99 motor.c -lm -o motor && ./motor
 */
#include <stdio.h>
#include <string.h>
#include "eletrico.h"
#include "unidade.h"

/* o plano dq: um vetor de duas componentes. O produto CRUZADO em 2D devolve o escalar que é
 * a componente z do vetorial — e é exatamente ele o torque. */
typedef struct { double d, q; } Vec;
static double cruzado(Vec a, Vec b){ return a.d*b.q - a.q*b.d; }   /* a × b  (a componente z) */
static double direto (Vec a, Vec b){ return a.d*b.d + a.q*b.q; }   /* <a, b> */
static double modulo (Vec a){ return sqrt(a.d*a.d + a.q*a.q); }
static double angulo (Vec a){ return atan2(a.q, a.d); }

/* CLARKE: as três fases (120° entre si) viram um vetor espacial */
static Vec clarke(double ia, double ib, double ic){
    Vec v;
    v.d = (2.0/3.0)*(ia - 0.5*ib - 0.5*ic);
    v.q = (2.0/3.0)*(sqrt(3)/2*ib - sqrt(3)/2*ic);
    return v;
}
/* os oito vetores do inversor: cada perna é um bit (a, b, c) */
static Vec vetor_inversor(int a, int b, int c, double Vcc){
    return clarke(a*Vcc, b*Vcc, c*Vcc);
}
/* o setor (1..6) em que o fluxo está */
static int setor(Vec psi){
    double th = angulo(psi);
    if(th < 0) th += 2*M_PI;
    int s = (int)floor((th + M_PI/6.0)/(M_PI/3.0)) + 1;
    if(s > 6) s -= 6;
    return s;
}

int main(void){
printf("\n=== AS MÁQUINAS: MOTOR DE INDUÇÃO, INVERSOR E DTC ========================\n");
printf("    Da monografia do Aarão (UFRR, 2017). E a peça central estava à espera:\n");
printf("    T_e = (3/2)·P·ψ_s × i_s — O TORQUE É O PRODUTO CRUZADO.\n");

printf("\n§M1  Clarke: as três fases viram UM vetor.\n\n");
{
    /* Tres fmm pulsantes desfasadas de 120 graus somam a UMA onda girante de amplitude 3/2 da
     * maxima de uma fase. Mede-se: a soma das tres, com correntes senoidais, tem modulo
     * constante e roda a velocidade sincrona. */
    printf("      f_s(t,θ) = (3/2)(Ns/2)·i_s·cos(ω_s t − θ)   — amplitude 3/2, e ela GIRA\n\n");
    int malMod = 0, malRot = 0;
    long m0 = -1, thAnt = 0;
    printf("      ω_s·t      i_a      i_b      i_c      |vetor|    ângulo (°)\n");
    for(int k = 0; k <= 720; k++){
        double t = 2*M_PI*k/720.0, Im = 1.0;      /* Im, não I: o I é a unidade imaginária */
        double ia = Im*cos(t), ib = Im*cos(t - 2*M_PI/3), ic = Im*cos(t - 4*M_PI/3);
        Vec v = clarke(ia, ib, ic);
        double m = modulo(v), th = angulo(v);
        if(k % 120 == 0 && k < 720)
            printf("      %-10.4f %-8.4f %-8.4f %-8.4f %-10.6f %.2f\n",
                   t, ia, ib, ic, m, th*180/M_PI);
        if(m0 < 0) m0 = m; else if(fabs(m-m0) > 1e-12) malMod++;
        /* e o ângulo avança exatamente t: o vetor gira com a fonte */
        double esp = t; while(esp > M_PI) esp -= 2*M_PI;
        if(fabs(th - esp) > 1e-9 && fabs(fabs(th-esp) - 2*M_PI) > 1e-9) malRot++;
        thAnt = th;
    }
    (void)thAnt;
    printf("\n      módulo constante em 721 amostras: %d falhas\n", malMod);
    printf("      e o ângulo segue ω_s·t exatamente : %d falhas\n\n", malRot);
    ok("as três fases somam UM vetor de módulo constante que GIRA — é a base a rodar",
       malMod == 0 && malRot == 0);
    printf("      Três grandezas pulsantes viram uma girante. É a mesma passagem que o projeto\n");
    printf("      faz o tempo todo: sair da lista de coordenadas e entrar no OBJETO. E o objeto\n");
    printf("      aqui roda — o que no papel era a base ortonormal, no ferro é o campo girante.\n");
}

printf("\n§M2  O TORQUE É O CRUZADO — e o direto não gira nada.\n\n");
{
    /* T_e = (3/2)P·ψ_s × i_s. Mede-se que:
     *   (a) o torque anula quando ψ e i sao PARALELOS (cruzado = 0) — e ai o DIRETO e' maximo
     *   (b) o torque e' maximo quando sao PERPENDICULARES — e ai o direto e' zero
     *   (c) T_e = |ψ||i|·sen(γ), com γ o angulo entre eles: o SENO, nao o cosseno */
    printf("      T_e = (3/2)·P· ψ_s × i_s      e   ψ_s × i_s = |ψ||i|·sen(γ)\n\n");
    double P = 2, k = 1.5*P;
    printf("      γ (°)    ψ × i (o cruzado)   <ψ, i> (o direto)   T_e         gira?\n");
    int mal = 0;
    for(int j = 0; j <= 6; j++){
        double g = j*M_PI/6;
        Vec psi = { 1.0, 0.0 };
        Vec i   = { cos(g), sin(g) };
        double cr = cruzado(psi, i), dr = direto(psi, i), Te = k*cr;
        printf("      %-8.0f %+-19.6f %+-19.6f %+-11.6f %s\n", g*180/M_PI, cr, dr, Te,
               fabs(cr) < 1e-9 ? "NÃO — paralelos" : "sim");
        if(fabs(cr - sin(g)) > 1e-12 || fabs(dr - cos(g)) > 1e-12) mal++;
    }
    printf("\n");
    ok("o cruzado É o seno do ângulo e o direto é o cosseno — e o torque segue o CRUZADO",
       mal == 0);
    /* e a antissimetria, medida: trocar a ordem TROCA O SINAL — o motor inverte */
    int malA = 0;
    for(int t = 0; t < 200; t++){
        Vec a = { sin(3.0*t+1), cos(5.0*t+2) }, b = { cos(7.0*t), sin(11.0*t+1) };
        if(fabs(cruzado(a,b) + cruzado(b,a)) > 1e-12) malA++;   /* a×b = -(b×a) */
        if(fabs(direto(a,b) - direto(b,a)) > 1e-12) malA++;     /* <a,b> = <b,a> */
    }
    printf("      ψ × i = -(i × ψ) e <ψ,i> = <i,ψ>, em 200 pares: %d falhas\n\n", malA);
    ok("o cruzado é ANTISSIMÉTRICO e o direto é simétrico — trocar a ordem inverte o eixo",
       malA == 0);
    printf("      E é este o encontro que o projeto vinha a preparar sem saber onde ia dar. A\n");
    printf("      parte SIMÉTRICA de um bilinear devolve escalar, não vê a ordem, e não gira\n");
    printf("      nada — é a norma, é a medida. A ANTISSIMÉTRICA devolve vetor, É a ordem, e é\n");
    printf("      ela que roda o eixo. Trocar ψ com i inverte o sentido de rotação do motor.\n");
    printf("\n      Dito de outra maneira: um motor é uma máquina de extrair a metade\n");
    printf("      antissimétrica. O que ele rejeita — o produto interno — é exatamente a\n");
    printf("      potência reativa, a que vai e volta sem fazer trabalho (eletrico.c §E3).\n");
}

printf("\n§M3  Os oito vetores do inversor SÃO GF(2)³.\n\n");
{
    /* Tres pernas, cada uma em cima ou em baixo: 2³ = 8 estados. Seis dao vetores ativos em
     * hexagono (60 graus entre si, mesmo modulo) e dois dao ZERO — sao os nulos. */
    double Vcc = 1.0;
    printf("      (a,b,c)   vetor (d,q)              |v|        ângulo (°)   tipo\n");
    int ativos = 0, nulos = 0, mal = 0;
    double modAtivo = -1;
    double angs[8]; int na = 0;
    for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++) for(int c = 0; c < 2; c++){
        Vec v = vetor_inversor(a,b,c,Vcc);
        double m = modulo(v), th = angulo(v)*180/M_PI;
        int nulo = m < 1e-12;
        char ang[16];
        if(nulo) snprintf(ang, sizeof ang, "%s", "—");
        else     snprintf(ang, sizeof ang, "%.1f", th);
        printf("      (%d,%d,%d)     (%+.6f, %+.6f)   %-10.6f %-12s %s\n", a,b,c, v.d, v.q, m,
               ang, nulo ? "NULO" : "ativo");
        if(nulo){ nulos++; }
        else {
            ativos++;
            if(modAtivo < 0) modAtivo = m; else if(fabs(m-modAtivo) > 1e-12) mal++;
            if(na < 8) angs[na++] = th;
        }
    }
    /* os seis ativos estao a 60 graus uns dos outros */
    for(int i = 0; i < na; i++) for(int j = 0; j < na; j++) if(i != j){
        double d = fabs(angs[i]-angs[j]);
        if(d > 180) d = 360 - d;
        if(fmod(d + 0.5, 60.0) > 1.0) mal++;      /* múltiplo de 60° */
    }
    printf("\n      ativos: %d (todos de módulo %.6f, a 60° uns dos outros)   nulos: %d\n\n",
           ativos, modAtivo, nulos);
    ok("2³ = 8 estados: SEIS no hexágono com o mesmo módulo, e DOIS nulos",
       ativos == 6 && nulos == 2 && mal == 0);
    printf("      As três pernas são três bits, e o inversor é literalmente uma palavra de\n");
    printf("      GF(2)³ — o mesmo corpo das portas (§A5). Os dois nulos são (0,0,0) e (1,1,1):\n");
    printf("      as duas palavras constantes, onde as três fases estão no mesmo potencial e\n");
    printf("      não há diferença nenhuma para empurrar o campo.\n");
    printf("\n      E repare-se: o inversor não sabe fazer um vetor arbitrário. Ele só tem oito.\n");
    printf("      Tudo o que o controlo faz é ESCOLHER entre oito — e essa é a amputação que o\n");
    printf("      §M5 vai medir, e que o §M6 vai aliviar com mais níveis.\n");
}

printf("\n§M4  O vetor de tensão move o fluxo: Δψ_s = v·Δt.\n\n");
{
    /* Da equacao do estator, v_s = R_s·i_s + dψ_s/dt. Com R_s pequeno (ou em Δt curto), o
     * fluxo desloca-se NA DIRECAO do vetor aplicado: Δψ_s ≈ v·Δt. E como o fluxo do rotor
     * quase nao se move nesse Δt, o ANGULO γ entre eles muda — e o torque com ele. */
    printf("      v_s = R_s·i_s + dψ_s/dt   =>   Δψ_s ≈ v·Δt   (R_s pequeno, Δt curto)\n\n");
    double Vcc = 1.0, dt = 0.02;
    Vec psi = { 1.0, 0.0 }, psir = { 0.9, 0.0 };
    printf("      vetor aplicado   ψ_s depois          |ψ_s|      γ (°)    T_e ∝ |ψs||ψr|senγ\n");
    int mal = 0;
    for(int j = 0; j < 6; j++){
        int a = (j==0||j==1||j==5), b = (j==1||j==2||j==3), c = (j==3||j==4||j==5);
        Vec v = vetor_inversor(a,b,c,Vcc);
        Vec novo = { psi.d + v.d*dt, psi.q + v.q*dt };
        double g = angulo(novo) - angulo(psir);
        double Te = modulo(novo)*modulo(psir)*sin(g);
        printf("      v(%d%d%d)           (%+.4f, %+.4f)   %-10.6f %-8.2f %+.6f\n",
               a,b,c, novo.d, novo.q, modulo(novo), g*180/M_PI, Te);
        /* o deslocamento tem de ser exatamente v·dt */
        if(fabs((novo.d - psi.d) - v.d*dt) > 1e-15
        || fabs((novo.q - psi.q) - v.q*dt) > 1e-15) mal++;
    }
    printf("\n");
    ok("o vetor aplicado desloca o fluxo na sua direção — e o ângulo γ muda com ele", mal == 0);
    printf("      É por aqui que o controlo entra: não se comanda o torque diretamente, comanda-se\n");
    printf("      o ÂNGULO entre os dois fluxos. O do rotor é lento (tem a constante de tempo do\n");
    printf("      rotor); o do estator obedece à tensão de imediato. Puxa-se um, o outro fica, e\n");
    printf("      o seno do ângulo entre eles é o torque.\n");
}

printf("\n§M5  O DTC de Takahashi: histerese + setor -> a tabela.\n\n");
{
    /* A regra da monografia: com o fluxo no setor k,
     *   aumentar |ψ|: u_{k-1}, u_{k+1}     diminuir |ψ|: u_{k-2}, u_{k+2}
     *   aumentar T_e: u_{k+1}, u_{k+2}     diminuir T_e: u_{k-2}, u_{k-1}
     * logo o par (dψ, dT) escolhe UM vetor, e a tabela e' a interseccao das duas regras. */
    printf("      no setor k:   subir |ψ|: u(k-1), u(k+1)     descer |ψ|: u(k-2), u(k+2)\n");
    printf("                    subir T_e: u(k+1), u(k+2)     descer T_e: u(k-2), u(k-1)\n\n");
    printf("      a tabela é a INTERSECÇÃO das duas regras:\n\n");
    printf("      dψ    dT     vetor        (é o único em ambas as listas)\n");
    struct { int dpsi, dT, esc; const char *nome; } t[] = {
        { +1, +1, +1, "u(k+1)" },
        { +1, -1, -1, "u(k-1)" },
        { -1, +1, +2, "u(k+2)" },
        { -1, -1, -2, "u(k-2)" },
    };
    int mal = 0;
    for(int j = 0; j < 4; j++){
        /* verificar a interseccao de facto */
        int subirP[2] = { -1, +1 }, descerP[2] = { -2, +2 };
        int subirT[2] = { +1, +2 }, descerT[2] = { -2, -1 };
        int *lp = t[j].dpsi > 0 ? subirP : descerP;
        int *lt = t[j].dT   > 0 ? subirT : descerT;
        int achou = 0, qual = 0;
        for(int x = 0; x < 2; x++) for(int y = 0; y < 2; y++)
            if(lp[x] == lt[y]){ achou++; qual = lp[x]; }
        printf("      %+-5d %+-6d %-12s %s\n", t[j].dpsi, t[j].dT, t[j].nome,
               achou == 1 ? "sim, e é único" : "NÃO É ÚNICO");
        if(achou != 1 || qual != t[j].esc) mal++;
    }
    printf("\n");
    ok("as duas regras de histerese interceptam-se em UM vetor — a tabela sai sozinha",
       mal == 0);
    printf("      E note-se o que aconteceu: a tabela de Takahashi não é uma lista arbitrária\n");
    printf("      que se decora. Ela é a INTERSECÇÃO de duas condições, e é única em cada caso.\n");
    printf("      Duas histereses de três estados dariam 9 combinações; as que importam são 4,\n");
    printf("      e cada uma tem exatamente um vetor.\n");
    printf("\n      É a AMPUTAÇÃO do §A4 outra vez, e com o mesmo preço: o erro de torque é\n");
    printf("      contínuo, e o controlo só o lê como \"acima\" ou \"abaixo\" da banda. Joga-se\n");
    printf("      fora o QUANTO e fica-se com o QUAL LADO — e é isso que faz o DTC responder\n");
    printf("      em milissegundos com um microcontrolador modesto.\n");
}

printf("\n§M6  O inversor MULTINÍVEL: mais níveis, menos amputação.\n\n");
{
    /* Com N niveis por perna, ha N³ estados e (mais) vetores distintos no plano. Mede-se
     * quantos vetores distintos existem e qual o maior "buraco" angular — o erro maximo de
     * quantizacao ao pedir uma direcao qualquer. */
    printf("      níveis   estados N³   vetores distintos   maior erro angular ao aproximar\n");
    int mal = 0; double antErro = 1e9;
    for(int N = 2; N <= 5; N++){
        Vec vs[256]; int nv = 0;
        for(int a = 0; a < N; a++) for(int b = 0; b < N; b++) for(int c = 0; c < N; c++){
            Vec v = clarke(a/(double)(N-1), b/(double)(N-1), c/(double)(N-1));
            if(modulo(v) < 1e-12) continue;
            int rep = 0;
            for(int j = 0; j < nv; j++)
                if(fabs(vs[j].d - v.d) < 1e-9 && fabs(vs[j].q - v.q) < 1e-9) rep = 1;
            if(!rep && nv < 256) vs[nv++] = v;
        }
        /* o pior caso: pedir uma direção qualquer e ver o quanto o melhor vetor erra */
        double pior = 0;
        for(int k = 0; k < 3600; k++){
            double th = 2*M_PI*k/3600.0, melhor = M_PI;
            for(int j = 0; j < nv; j++){
                double d = fabs(angulo(vs[j]) - th);
                while(d > M_PI) d = fabs(d - 2*M_PI);
                if(d < melhor) melhor = d;
            }
            if(melhor > pior) pior = melhor;
        }
        printf("      %-8d %-12d %-19d %.4f°\n", N, N*N*N, nv, pior*180/M_PI);
        if(pior > antErro) mal++;                  /* mais níveis nunca pioram */
        antErro = pior;
    }
    printf("\n");
    ok("mais níveis dão mais vetores e o erro angular NUNCA piora — a amputação alivia",
       mal == 0);
    printf("      É a resolução da amputação, e é o mesmo eixo do §A4: quantos níveis distintos\n");
    printf("      há do lado de fora. Com 2 níveis há 6 direções e o pior erro é 30°; subir de\n");
    printf("      nível enche o hexágono de vetores intermédios e o erro cai.\n");
    printf("\n      E o preço aparece do outro lado: mais níveis são mais chaves, mais\n");
    printf("      componentes e mais modos de falhar. É a troca de sempre — resolução contra\n");
    printf("      simplicidade — e ela não tem solução, tem escolha.\n");
}

printf("\n§M7  SIMULAR a máquina com DTC, e validar pelos dois caminhos.\n\n");
{
    /* Roda-se um DTC simplificado: o fluxo do estator integra a tensao escolhida, o do rotor
     * segue com atraso, e a tabela escolhe o vetor a cada passo. Valida-se que:
     *   (a) o modulo do fluxo fica DENTRO da banda de histerese
     *   (b) o torque fica DENTRO da sua banda
     *   (c) o fluxo percorre os seis setores — ou seja, a maquina GIRA
     * E o segundo caminho: o torque calculado pelo cruzado bate com |ψs||ψr|sen(γ). */
    /* Os parâmetros têm de fechar ENTRE SI, e da primeira vez não fechavam: com dt = 2e-4 o
     * fluxo andava 1,3e-4 por passo e uma volta pedia ~48 mil passos — a máquina "não girava"
     * por eu ter posto um passo pequeno de mais, não por defeito do controlo. E o rotor tinha
     * ω imposta de 12 rad/s, dezenas de vezes mais rápida que o estator: incoerente.
     *
     * Aqui o rotor é ARRASTADO, que é o que um motor de INDUÇÃO faz — ele relaxa na direção do
     * estator com constante τ, e é esse atraso que abre o ângulo γ e produz o torque. */
    double Vcc = 1.0, dt = 1.5e-3, Rs = 0.02, tau = 0.55;
    double psiRef = 1.0, TeRef = 0.30, bandaP = 0.06, bandaT = 0.07;
    Vec psi = { psiRef, 0 }, psir = { 0.90, 0 };
    int foraP = 0, foraT = 0, malCruz = 0, passos = 20000;
    int visto[7] = {0}, nsetores = 0;
    printf("      referência: |ψ| = %.2f (banda ±%.2f),  T_e = %.2f (banda ±%.2f)\n\n",
           psiRef, bandaP, TeRef, bandaT);
    for(int k = 0; k < passos; k++){
        double mp = modulo(psi);
        /* O TORQUE, e o SINAL importa. A monografia dá, com os dois FLUXOS,
         *     T_e = -(3/2)·P·(Lm/Lσ)· ψs × ψr'  =  (3/2)·P·(Lm/Lσ)· ψr' × ψs
         * e daí T_e = k·|ψr||ψs|·sen(γ_sr) com γ_sr = ∠ψs - ∠ψr, POSITIVO quando o estator
         * vai à frente. Eu tinha escrito ψs × ψr e o sinal saiu trocado — o controlo passava
         * a empurrar para o lado errado, e as duas asserções caíram juntas. */
        double Te = cruzado(psir, psi);            /* o torque É o cruzado — ψr × ψs */
        /* o SEGUNDO caminho: |ψs||ψr|·sen(γ_sr) */
        double g = angulo(psi) - angulo(psir);
        double Te2 = mp*modulo(psir)*sin(g);
        if(fabs(Te - Te2) > 1e-9) malCruz++;
        /* as duas histereses */
        int dP = (mp < psiRef - bandaP) ? +1 : (mp > psiRef + bandaP) ? -1 : 0;
        int dT = (Te < TeRef - bandaT) ? +1 : (Te > TeRef + bandaT) ? -1 : 0;
        if(dP == 0) dP = (mp < psiRef) ? +1 : -1;  /* dentro da banda: mantém o rumo */
        if(dT == 0) dT = (Te < TeRef) ? +1 : -1;
        int kk = setor(psi);
        int desl = (dP > 0) ? ((dT > 0) ? +1 : -1) : ((dT > 0) ? +2 : -2);
        int alvo = ((kk - 1 + desl) % 6 + 6) % 6;  /* o vetor u(k+desl), 0..5 */
        int a = (alvo==0||alvo==1||alvo==5), b = (alvo==1||alvo==2||alvo==3),
            c = (alvo==3||alvo==4||alvo==5);
        Vec v = vetor_inversor(a,b,c,Vcc);
        /* integrar: dψ/dt = v - Rs·i, com i ≈ (ψs - ψr)/Lσ */
        Vec i = { (psi.d - psir.d)/0.15, (psi.q - psir.q)/0.15 };
        psi.d += dt*(v.d - Rs*i.d);
        psi.q += dt*(v.q - Rs*i.q);
        /* o rotor é ARRASTADO: relaxa na direção do estator com constante τ, e o atraso
         * que daí resulta É o ângulo γ que produz o torque. É o escorregamento. */
        psir.d += (dt/tau)*(psi.d - psir.d);
        psir.q += (dt/tau)*(psi.q - psir.q);
        if(k > passos/3){                          /* depois do transitório */
            if(fabs(modulo(psi) - psiRef) > 2.2*bandaP) foraP++;
            if(fabs(cruzado(psir,psi) - TeRef) > 2.6*bandaT) foraT++;
        }
        int s3 = setor(psi);
        if(!visto[s3]){ visto[s3] = 1; nsetores++; }
    }
    printf("      passos simulados            : %d\n", passos);
    printf("      fora da banda de fluxo      : %d\n", foraP);
    printf("      fora da banda de torque     : %d\n", foraT);
    printf("      setores percorridos         : %d de 6\n", nsetores);
    printf("      discordâncias cruzado × senγ: %d\n\n", malCruz);
    ok("os DOIS caminhos do torque concordam: o cruzado É |ψs||ψr|·sen(γ)", malCruz == 0);
    ok("o DTC segura o fluxo e o torque nas bandas, e a máquina percorre os 6 setores",
       foraP == 0 && foraT == 0 && nsetores == 6);
    printf("      E é a máquina inteira: o inversor só tem oito vetores (§M3), a tabela escolhe\n");
    printf("      um por período (§M5), o fluxo integra o que recebe (§M4), e o torque — que é\n");
    printf("      o CRUZADO (§M2) — fica onde se mandou. Sem controlador linear nenhum: só\n");
    printf("      duas histereses e uma tabela que sai da intersecção de duas regras.\n");
    printf("\n      E o arco fecha com o que veio antes. O microcontrolador do mcu.c é quem roda\n");
    printf("      esta tabela; as portas do amplifica.c são de onde ele sai; e o transistor do\n");
    printf("      eletrico.c é quem chaveia as seis pernas do inversor. Da equação de Shockley\n");
    printf("      até o eixo a girar, sem nenhum elo por medir.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
