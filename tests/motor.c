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
 *   §M1  Clarke: as três fases viram UM vetor — órbita de período 6, ‖v‖² constante
 *   §M2  o TORQUE É O CRUZADO — paralelo anula, perp é máximo, Lagrange em ℤ
 *   §M3  os oito vetores do inversor SÃO GF(2)³: seis no hexágono, dois nulos
 *   §M4  o vetor de tensão move o fluxo: Δψ = v (dt = 1), e a regra de |ψ| é a tabela
 *   §M5  o DTC de Takahashi: histerese + setor -> a tabela, e é a amputação
 *   §M6  o inversor MULTINÍVEL: os pares de Clarke são o hexagonal 3N(N−1)+1
 *   §M7  a máquina com DTC: Lagrange do torque + órbita de período 24 nos 6 setores
 *
 * LEI vs TRANSPORTE. Clarke com sin/cos em 721 amostras, atan2, sqrt, 20 000 passos de
 * vírgula e 1e-9 no cruzado eram o método. A lei é a forma p²+3s² (Clarke sem √3 formado),
 * o hexágono GF(2)³ de norma 4, Lagrange cruz²+dir²=‖a‖²‖b‖², a intersecção da tabela, o
 * contagem 3N(N−1)+1, e o DTC como órbita inteira — sem uma raiz, sem um ângulo.
 *
 *   cc -O2 -std=c99 -I lib tests/motor.c -o motor && ./motor
 */
#include <stdio.h>
#include "unidade.h"

/* Clarke em ℤ: p = 2ia−ib−ic, s = ib−ic. A forma ‖v‖²_Clarke · 9 = p² + 3s²
 * (o √3 de v_q fica no 3 da forma, e não se extrai). */
typedef struct { long p, s; } Vec;
static long n2(Vec a){ return a.p*a.p + 3*a.s*a.s; }
static long dirC(Vec a, Vec b){ return a.p*b.p + 3*a.s*b.s; }
static long cruz(Vec a, Vec b){ return a.p*b.s - a.s*b.p; }     /* vezes √3 no plano dq */
static Vec clarke(long ia, long ib, long ic){
    Vec v; v.p = 2*ia - ib - ic; v.s = ib - ic; return v;
}

/* os seis ativos, em ordem de 60°: u1=(100) … u6=(101) */
static const int UA[6] = {1,1,0,0,0,1};
static const int UB[6] = {0,1,1,1,0,0};
static const int UC[6] = {0,0,0,1,1,1};
static Vec u_ativo(int j){
    j = ((j % 6) + 6) % 6;
    return clarke(UA[j], UB[j], UC[j]);
}
/* setor = argmax_j ⟨ψ, u_j⟩_Clarke — o cone de 60°, sem atan2 */
static int setor(Vec psi){
    int best = 0; long bestip = dirC(psi, u_ativo(0));
    for(int j = 1; j < 6; j += 1){
        long ip = dirC(psi, u_ativo(j));
        if(ip > bestip){ bestip = ip; best = j; }
    }
    return best;
}

int main(void){
printf("\n=== AS MÁQUINAS: MOTOR DE INDUÇÃO, INVERSOR E DTC ========================\n");
printf("    Da monografia do Aarão (UFRR, 2017). E a peça central estava à espera:\n");
printf("    T_e = (3/2)·P·ψ_s × i_s — O TORQUE É O PRODUTO CRUZADO.\n");

printf("\n§M1  Clarke: as três fases viram UM vetor.\n\n");
{
    /* Três fmm desfasadas de 120° somam UMA girante. Em ℤ: a amostra (2,−1,−1) é
     * 2·(cos 0, cos 120°, cos 240°), e a rotação de 60° é (a,b,c) ↦ (−b,−c,−a),
     * período 6. A forma p²+3s² é a mesma nos seis — o módulo NÃO oscila. */
    printf("      f_s(t,θ) = (3/2)(Ns/2)·i_s·cos(ω_s t − θ)   — amplitude 3/2, e ela GIRA\n\n");
    printf("      passo    i_a   i_b   i_c     p     s     p²+3s²\n");
    long ia = 2, ib = -1, ic = -1;
    long n0 = n2(clarke(ia,ib,ic));
    int malMod = 0, malPer = 0, distintos = 0;
    long visto_p[6], visto_s[6];
    long ia0 = ia, ib0 = ib, ic0 = ic;
    for(int k = 0; k < 6; k += 1){
        Vec v = clarke(ia, ib, ic);
        printf("      %-8d %-5ld %-5ld %-5ld %-5ld %-5ld %ld\n",
               k, ia, ib, ic, v.p, v.s, n2(v));
        if(n2(v) != n0) malMod++;
        if(ia + ib + ic != 0) malMod++;
        int rep = 0;
        for(int j = 0; j < distintos; j += 1)
            if(visto_p[j] == v.p && visto_s[j] == v.s) rep = 1;
        if(!rep){ visto_p[distintos] = v.p; visto_s[distintos] = v.s; distintos++; }
        long na = -ib, nb = -ic, nc = -ia;          /* rotação de 60° */
        ia = na; ib = nb; ic = nc;
    }
    if(ia != ia0 || ib != ib0 || ic != ic0) malPer++;
    printf("\n      módulo p²+3s² = %ld nos 6 passos, %d distintos, volta ao início: %s\n\n",
           n0, distintos, malPer ? "nao" : "sim");
    ok("as três fases somam UM vetor de módulo constante que GIRA — é a base a rodar."
       " 721 senos mediam IEEE; a órbita (2,-1,-1) sob (a,b,c)|->(-b,-c,-a) tem período 6"
       " e p²+3s² constante, exacto em Z",
       malMod == 0 && malPer == 0 && distintos == 6 && n0 == 36);
    printf("      Três grandezas pulsantes viram uma girante. É a mesma passagem que o projeto\n");
    printf("      faz o tempo todo: sair da lista de coordenadas e entrar no OBJETO. E o objeto\n");
    printf("      aqui roda — o que no papel era a base ortonormal, no ferro é o campo girante.\n");
}

printf("\n§M2  O TORQUE É O CRUZADO — e o direto não gira nada.\n\n");
{
    printf("      T_e = (3/2)·P· ψ_s × i_s      e   ψ_s × i_s = |ψ||i|·sen(γ)\n\n");
    printf("      caso         cruzado     directo     gira?\n");
    Vec psi = { 5, 0 };
    Vec par = { 3, 0 }, perp = { 0, 1 };
    long cr_par = cruz(psi, par),  dr_par = dirC(psi, par);
    long cr_pe  = cruz(psi, perp), dr_pe  = dirC(psi, perp);
    printf("      paralelos    %-11ld %-11ld %s\n", cr_par, dr_par,
           cr_par == 0 ? "NAO — paralelos" : "sim");
    printf("      perpendic.   %-11ld %-11ld %s\n\n", cr_pe, dr_pe,
           cr_pe == 0 ? "NAO — paralelos" : "sim");
    /* Lagrange na forma de Clarke: dir² + 3 cruz² = ‖a‖² ‖b‖² */
    int malL = 0, totL = 0;
    for(long a0 = -4; a0 <= 4; a0 += 1) for(long a1 = -4; a1 <= 4; a1 += 1)
    for(long b0 = -4; b0 <= 4; b0 += 1) for(long b1 = -4; b1 <= 4; b1 += 1){
        Vec a = { a0, a1 }, b = { b0, b1 };
        long esq = dirC(a,b)*dirC(a,b) + 3*cruz(a,b)*cruz(a,b);
        long dir = n2(a)*n2(b);
        totL++;
        if(esq != dir) malL++;
    }
    printf("      Lagrange dir² + 3 cruz² = ‖a‖²‖b‖² em %d pares: %d falhas\n", totL, malL);
    ok("o cruzado anula nos paralelos e é máximo nos perpendiculares — e Lagrange fecha"
       " em Z, na forma de Clarke: o seno e o cosseno eram 1e-12 sobre sin/cos; a identidade"
       " e' polinomial",
       cr_par == 0 && dr_par != 0 && cr_pe != 0 && dr_pe == 0
       && malL == 0 && totL > 0);

    int malA = 0, totA = 0;
    for(long a0 = -3; a0 <= 3; a0 += 1) for(long a1 = -3; a1 <= 3; a1 += 1)
    for(long b0 = -3; b0 <= 3; b0 += 1) for(long b1 = -3; b1 <= 3; b1 += 1){
        Vec a = { a0, a1 }, b = { b0, b1 };
        totA++;
        if(cruz(a,b) + cruz(b,a) != 0) malA++;
        if(dirC(a,b) - dirC(b,a) != 0) malA++;
    }
    printf("      ψ × i = -(i × ψ) e ⟨ψ,i⟩ = ⟨i,ψ⟩, em %d pares: %d falhas\n\n", totA, malA);
    ok("o cruzado é ANTISSIMÉTRICO e o direto é simétrico — trocar a ordem inverte o eixo."
       " 200 pares de sin/cos mediam IEEE; a antissimetria e' a definição bilinear em Z",
       malA == 0 && totA > 0);
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
    printf("      (a,b,c)   (p,s)        p²+3s²    tipo\n");
    int ativos = 0, nulos = 0, mal = 0;
    long nAtivo = -1;
    for(int a = 0; a < 2; a += 1) for(int b = 0; b < 2; b += 1) for(int c = 0; c < 2; c += 1){
        Vec v = clarke(a, b, c);
        long m = n2(v);
        int nulo = (v.p == 0 && v.s == 0);
        printf("      (%d,%d,%d)     (%+ld,%+ld)      %-8ld %s\n",
               a, b, c, v.p, v.s, m, nulo ? "NULO" : "ativo");
        if(nulo) nulos++;
        else {
            ativos++;
            if(nAtivo < 0) nAtivo = m;
            else if(m != nAtivo) mal++;
        }
    }
    /* adjacentes do hexágono: ⟨u_j, u_{j+1}⟩ = 2 = n2/2  (cos 60° exacto) */
    int malHex = 0;
    for(int j = 0; j < 6; j += 1){
        if(dirC(u_ativo(j), u_ativo(j+1)) != 2) malHex++;
        if(dirC(u_ativo(j), u_ativo(j+3)) != -4) malHex++;   /* opostos: cos 180° = −1 */
        if(n2(u_ativo(j)) != 4) malHex++;
    }
    printf("\n      ativos: %d (todos com p²+3s² = %ld, adjacentes ⟨ , ⟩ = 2)   nulos: %d\n\n",
           ativos, nAtivo, nulos);
    ok("2³ = 8 estados: SEIS no hexágono com o mesmo módulo, e DOIS nulos."
       " O 1e-12 no |v| e o fmod de 60° eram transporte; a forma p²+3s² = 4 nos seis"
       " e ⟨u_j, u_{j+1}⟩ = 2 e' o cos 60° sem um cosseno",
       ativos == 6 && nulos == 2 && mal == 0 && malHex == 0 && nAtivo == 4);
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
    printf("      v_s = R_s·i_s + dψ_s/dt   =>   Δψ_s = v     (dt = 1, R_s = 0 neste passo)\n\n");
    Vec psi = { 6, 0 };                     /* sector 0, n2 = 36 */
    printf("      vetor     ψ depois      p²+3s²    |ψ| sobe?\n");
    int mal = 0;
    long nAntes = n2(psi);
    /* no setor k=0: subir |ψ| é u(k±1); descer é u(k±2) — e dt=1 é soma em Z */
    int sobe[2] = { 1, 5 }, desce[2] = { 2, 4 };
    for(int t = 0; t < 2; t += 1){
        Vec v = u_ativo(sobe[t]);
        Vec novo = { psi.p + v.p, psi.s + v.s };
        printf("      u(k%+d)    (%+ld,%+ld)      %-8ld %s\n",
               (sobe[t] == 1) ? +1 : -1, novo.p, novo.s, n2(novo),
               n2(novo) > nAntes ? "sobe" : "desce");
        if(n2(novo) <= nAntes) mal++;
    }
    for(int t = 0; t < 2; t += 1){
        Vec v = u_ativo(desce[t]);
        Vec novo = { psi.p + v.p, psi.s + v.s };
        printf("      u(k%+d)    (%+ld,%+ld)      %-8ld %s\n",
               (desce[t] == 2) ? +2 : -2, novo.p, novo.s, n2(novo),
               n2(novo) > nAntes ? "sobe" : "desce");
        if(n2(novo) >= nAntes) mal++;
    }
    printf("\n");
    ok("o vetor aplicado desloca o fluxo na sua direção — e a regra de Takahashi sobre |ψ|"
       " mede-se na forma: u(k±1) SOBE p²+3s², u(k±2) DESCE, exacto em Z, sem atan2",
       mal == 0 && nAntes == 36);
    printf("      É por aqui que o controlo entra: não se comanda o torque diretamente, comanda-se\n");
    printf("      o ÂNGULO entre os dois fluxos. O do rotor é lento (tem a constante de tempo do\n");
    printf("      rotor); o do estator obedece à tensão de imediato. Puxa-se um, o outro fica, e\n");
    printf("      o seno do ângulo entre eles é o torque.\n");
}

printf("\n§M5  O DTC de Takahashi: histerese + setor -> a tabela.\n\n");
{
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
    for(int j = 0; j < 4; j += 1){
        int subirP[2] = { -1, +1 }, descerP[2] = { -2, +2 };
        int subirT[2] = { +1, +2 }, descerT[2] = { -2, -1 };
        int *lp = t[j].dpsi > 0 ? subirP : descerP;
        int *lt = t[j].dT   > 0 ? subirT : descerT;
        int achou = 0, qual = 0;
        for(int x = 0; x < 2; x += 1) for(int y = 0; y < 2; y += 1)
            if(lp[x] == lt[y]){ achou++; qual = lp[x]; }
        printf("      %+-5d %+-6d %-12s %s\n", t[j].dpsi, t[j].dT, t[j].nome,
               achou == 1 ? "sim, e é único" : "nao e unico");
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
    /* N níveis por perna → N³ estados. Os pares (p,s) distintos são o número HEXAGONAL
     * 3N(N−1)+1 (incluindo o nulo). Mais N nunca diminui — e aqui SOBE, exacto. */
    printf("      níveis   estados N³   vetores distintos   3N(N−1)+1\n");
    int mal = 0; long ant = 0;
    for(int N = 2; N <= 5; N += 1){
        long ps[125][2]; int nv = 0;
        for(int a = 0; a < N; a += 1) for(int b = 0; b < N; b += 1) for(int c = 0; c < N; c += 1){
            Vec v = clarke(a, b, c);
            int rep = 0;
            for(int j = 0; j < nv; j += 1)
                if(ps[j][0] == v.p && ps[j][1] == v.s) rep = 1;
            if(!rep && nv < 125){ ps[nv][0] = v.p; ps[nv][1] = v.s; nv++; }
        }
        long hex = 3L*N*(N-1) + 1;
        printf("      %-8d %-12d %-19d %ld\n", N, N*N*N, nv, hex);
        if(nv != hex) mal++;
        if(N > 2 && nv <= ant) mal++;
        ant = nv;
    }
    printf("\n");
    ok("mais níveis dão mais vetores e o erro NUNCA piora — a amputação alivia."
       " Os 3600 ângulos e o pior erro em graus eram transporte; os pares (p,s)"
       " distintos SÃO o hexagonal 3N(N-1)+1, exacto em Z",
       mal == 0);
    printf("      É a resolução da amputação, e é o mesmo eixo do §A4: quantos níveis distintos\n");
    printf("      há do lado de fora. Com 2 níveis há 7 pontos de Clarke (6 direções + nulo);\n");
    printf("      subir de nível enche o hexágono de vetores intermédios.\n");
    printf("\n      E o preço aparece do outro lado: mais níveis são mais chaves, mais\n");
    printf("      componentes e mais modos de falhar. É a troca de sempre — resolução contra\n");
    printf("      simplicidade — e ela não tem solução, tem escolha.\n");
}

printf("\n§M7  SIMULAR a máquina com DTC, e validar pelos dois caminhos.\n\n");
{
    /* Os 20 000 passos em vírgula mediam bandas IEEE. A lei: (i) os DOIS caminhos do
     * torque concordam por Lagrange; (ii) o DTC com histerese em p²+3s² é uma ÓRBITA
     * de período 24 que visita os 6 setores e regressa a (6,0). */
    printf("      referência: p²+3s² = 36,  histerese: sobe se ≤ 36, desce se > 36\n");
    printf("      torque sempre a SUBIR (dT = +1) — a máquina GIRA\n\n");

    int malCruz = 0, totC = 0;
    for(long a0 = -3; a0 <= 3; a0 += 1) for(long a1 = -3; a1 <= 3; a1 += 1)
    for(long b0 = -3; b0 <= 3; b0 += 1) for(long b1 = -3; b1 <= 3; b1 += 1){
        Vec a = { a0, a1 }, b = { b0, b1 };
        long esq = dirC(a,b)*dirC(a,b) + 3*cruz(a,b)*cruz(a,b);
        totC++;
        if(esq != n2(a)*n2(b)) malCruz++;
    }
    printf("      cruzado × senγ  (Lagrange na forma de Clarke): %d falhas em %d pares\n",
           malCruz, totC);
    ok("os DOIS caminhos do torque concordam: o cruzado É |ψs||ψr|·sen(γ)."
       " 1e-9 sobre atan2/sin era IEEE; dir² + 3 cruz² = ‖ψ‖²‖i‖² e' a identidade",
       malCruz == 0 && totC > 0);

    Vec psi = { 6, 0 };
    long nRef = 36;
    int visto[6] = {0}, nsetores = 0, malOrb = 0;
    long nMin = n2(psi), nMax = n2(psi);
    printf("      passo   setor   dψ    (p,s)        p²+3s²\n");
    int passos = 24;
    for(int k = 0; k < passos; k += 1){
        int kk = setor(psi);
        if(!visto[kk]){ visto[kk] = 1; nsetores++; }
        int dP = (n2(psi) <= nRef) ? +1 : -1;
        int desl = (dP > 0) ? +1 : +2;              /* dT = +1: tabela M5 */
        Vec v = u_ativo(kk + desl);
        psi.p += v.p; psi.s += v.s;
        long m = n2(psi);
        if(m < nMin) nMin = m;
        if(m > nMax) nMax = m;
        if(k < 8 || k == passos - 1)
            printf("      %-7d %-7d %+-5d (%+ld,%+ld)      %ld\n",
                   k, kk+1, dP, psi.p, psi.s, m);
        else if(k == 8) printf("      ...\n");
    }
    if(psi.p != 6 || psi.s != 0) malOrb++;
    printf("\n      passos                       : %d\n", passos);
    printf("      setores percorridos           : %d de 6\n", nsetores);
    printf("      p²+3s² na banda               : [%ld, %ld]\n", nMin, nMax);
    printf("      regressa a (6,0) após 24      : %s\n\n", malOrb ? "nao" : "sim");
    ok("o DTC segura o fluxo na banda e a máquina percorre os 6 setores — órbita de"
       " período 24 em Z, p²+3s² entre 28 e 52, sem 20 000 passos de vírgula",
       malOrb == 0 && nsetores == 6 && nMin >= 28 && nMax <= 52);
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
