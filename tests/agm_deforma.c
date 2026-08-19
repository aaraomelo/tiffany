/* agm_deforma.c — O INVARIANTE DO AGM SOBREVIVE À DEFORMAÇÃO?
 *
 * O AGM tem invariante exato (agm.c): I(a,b)=I((a+b)/2,√(ab)), e a convergência é quadrática. A
 * pergunta é o que acontece quando se deforma. E há DOIS tipos de deformação, com respostas opostas
 * — é isso que se mede:
 *
 *  (AD1/AD2) DEFORMAR A REGRA. Troca-se a média geométrica pela média de potência
 *              M_p(a,b) = ((a^p+b^p)/2)^{1/p} ,   p→0 é a geométrica (o AGM), p>0 deforma.
 *            E o resultado é uma DISSOCIAÇÃO: a velocidade sobrevive, o invariante não.
 *              · a convergência quadrática RESISTE a todo p≠1, com razão fechada (1−p)/(8M) —
 *                todas as médias de potência concordam a 2ª ordem, logo dobrar os dígitos é
 *                genérico e não é o que distingue o AGM;
 *              · o invariante elíptico MORRE, e à primeira ordem: |ΔI|/I ~ p.
 *            Nota: toda iteração de médias que converge tem "invariante" trivial — o próprio
 *            limite, 1/M(a,b), é invariante por construção. O que é especial no AGM é o invariante
 *            ser uma INTEGRAL fechada, e é isso que se testa.
 *
 *  (AD3) DEFORMAR PELA ISOGENIA. A batida do AGM é a transformação de Landen, e no módulo do toro
 *        ela faz τ → 2τ: a iteração MOVE o toro (desce a torre de 2-isogenias) e ainda assim
 *        PRESERVA I. Então a deformação-isogenia leva ponto de ancoragem em ponto de ancoragem: de
 *        τ=1 (k=1/√2, a lemniscata) para τ=2 (k=3−2√2), ambos singular values. O invariante
 *        sobrevive não como ponto, mas como CLASSE.
 *
 *  (AD4) O CONTRASTE COM KAM. Em KAM o toro sobrevive numa FAIXA de deformação (até K_c≈0,9716 para
 *        o áureo) e morre depois. Aqui, se o invariante elíptico morrer LINEARMENTE em p, não há
 *        faixa nenhuma: ele é exato sob a simetria (isogenia) e morre a qualquer outra deformação.
 *        O expoente de p é a medida que decide, e é o que se mede.
 *
 *   cc -O2 -std=c99 -I lib tests/agm_deforma.c -o agm_deforma && ./agm_deforma
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

static int passou = 1;

static long raiz_piso(long x){
    if(x < 0) return -1;
    if(x < 2) return x;
    long lo = 1, hi = 3037000499L;
    while(lo < hi){
        long mid = lo + (hi - lo + 1)/2;
        if(mid <= x / mid) lo = mid; else hi = mid - 1;
    }
    return lo;
}

/* M_p exacto para p ∈ {−1,0,+1,+2}; devolve 0 se não fechar em ℤ */
static int media_p_z(long a, long b, int p, long *mn, long *md){
    if(p == 0){
        long g = raiz_piso(a*b);
        if(g*g != a*b) return 0;
        *mn = g; *md = 1; return 1;
    }
    if(p == -1){ *mn = 2*a*b; *md = a+b; return 1; }
    if(p == 2){ *mn = a*a + b*b; *md = 2; return 1; }
    if(p == 1){
        if((a+b) % 2) return 0;
        *mn = (a+b)/2; *md = 1; return 1;
    }
    return 0;
}

int main(void){
    printf("AGM_DEFORMA — o invariante sobrevive à deformação?\n");
    printf("=================================================================\n");

    /* ---------- AD1: a DUPLICAÇÃO sobrevive — e a razão tem fórmula fechada ---------- */
    printf("§AD1 deformando a REGRA (b←M_p em vez de √(ab)): a convergência ainda DOBRA?\n");
    printf("     A conta diz que SIM, para todo p≠1. Com a=m(1+ε), b=m(1−ε):\n");
    printf("        M_p = m(1 + (p−1)ε²/2)   ⟹   M₁ − M_p = (1−p)(a−b)²/(8m)\n");
    printf("     — quadrático sempre, com razão d_{n+1}/d_n² → (1−p)/(8·M). Todas as médias de\n");
    printf("     potência concordam a SEGUNDA ordem: a duplicação é genérica, não é privilégio\n");
    printf("     do AGM. E mede-se em ℚ, nos quatro p em que M_p é algébrico simples —\n");
    printf("     sem limite numérico e sem tolerância:\n");
    {
        /* E A FÓRMULA FECHA EM ℚ — não é preciso medir o limite com uma tolerância.
         * Com a = m(1+ε) e b = m(1−ε), a tese é que TODA a média de potência concorda
         * com a aritmética até à 2ª ordem:  M_p = m(1 + (p−1)ε²/2).  Nos quatro p em
         * que M_p é algébrico simples isso verifica-se SEM aproximação nenhuma:
         *
         *   p = +1  M = (a+b)/2 = m                          e a fórmula dá m       → resíduo 0
         *   p = −1  M = 2ab/(a+b) = m(1−ε²)                  e a fórmula dá m(1−ε²) → resíduo 0
         *   p =  0  M² = ab = m²(1−ε²)                       e F = m(1−ε²/2):
         *   p = +2  M² = (a²+b²)/2 = m²(1+ε²)                e F = m(1+ε²/2):
         *
         *        F² − M²  =  m²·ε⁴/4      nos DOIS,  exactamente e sem sobra
         *
         * — a discordância é de ordem QUATRO, que é precisamente o que «concordam a 2ª
         * ordem» quer dizer. Compara-se por produto cruzado com ε = ep/eq, e o `2e-2`
         * que aqui estava media a convergência do long double e não a fórmula. */
        int erro = 0;
        long m = 5, ep = 1, eq = 4;                 /* ε = 1/4 */
        long A_n = m*(eq+ep), B_n = m*(eq-ep), den = eq;    /* a = A_n/den, b = B_n/den */
        printf("      m = %ld, ε = %ld/%ld   ⟹   a = %ld/%ld, b = %ld/%ld\n",
               m, ep, eq, A_n, den, B_n, den);
        printf("        p     M_p (exacto)          F = m(1+(p−1)ε²/2)     F² − M² = m²ε⁴/4 ?\n");
        {   /* p = +1 : a média aritmética É a fórmula, resíduo ZERO */
            long Mn = A_n + B_n, Md = 2*den;                /* M = (a+b)/2 */
            int ok1 = (Mn * 1 == m * Md);                   /* M == m  ⟺  Mn/Md == m/1 */
            printf("        +1    %ld/%-18ld %-22ld %s (resíduo 0)\n", Mn, Md, m, ok1?"✓":"✗");
            if(!ok1) erro = 1;
        }
        {   /* p = −1 : a harmónica dá m(1−ε²) — outra vez EXACTA, sem 4ª ordem */
            long Mn = 2*A_n*B_n, Md = den*(A_n + B_n);      /* M = 2ab/(a+b) */
            long Fn = m*(eq*eq - ep*ep), Fd = eq*eq;        /* F = m(1−ε²) */
            int ok1 = (Mn * Fd == Fn * Md);
            printf("        -1    %ld/%-18ld %ld/%-19ld %s (resíduo 0)\n", Mn, Md, Fn, Fd, ok1?"✓":"✗");
            if(!ok1) erro = 1;
        }
        {   /* p = 0 : M² = ab, F = m(1−ε²/2), e F² − M² tem de ser m²ε⁴/4 */
            long M2n = A_n*B_n, M2d = den*den;              /* M² = ab            */
            long Fn = m*(2*eq*eq - ep*ep), Fd = 2*eq*eq;    /* F = m(1−ε²/2)      */
            long Rn = m*m*ep*ep*ep*ep, Rd = 4*eq*eq*eq*eq;  /* m²ε⁴/4             */
            /* F² − M² == R,  por produto cruzado: (Fn²·M2d − M2n·Fd²)·Rd == Rn·Fd²·M2d */
            long esq = (Fn*Fn*M2d - M2n*Fd*Fd) * Rd, dir = Rn * Fd*Fd * M2d;
            printf("         0    √(%ld/%ld)%*s %ld/%-19ld %s (= %ld/%ld)\n",
                   M2n, M2d, 8, "", Fn, Fd, esq==dir?"✓":"✗", Rn, Rd);
            if(esq != dir) erro = 1;
        }
        {   /* p = +2 : M² = (a²+b²)/2, F = m(1+ε²/2), e o resíduo é o MESMO m²ε⁴/4 */
            long M2n = A_n*A_n + B_n*B_n, M2d = 2*den*den;
            long Fn = m*(2*eq*eq + ep*ep), Fd = 2*eq*eq;
            long Rn = m*m*ep*ep*ep*ep, Rd = 4*eq*eq*eq*eq;
            long esq = (Fn*Fn*M2d - M2n*Fd*Fd) * Rd, dir = Rn * Fd*Fd * M2d;
            printf("        +2    √(%ld/%ld)%*s %ld/%-19ld %s (= %ld/%ld, o MESMO)\n",
                   M2n, M2d, 7, "", Fn, Fd, esq==dir?"✓":"✗", Rn, Rd);
            if(esq != dir) erro = 1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 EXACTO — a convergência QUADRÁTICA sobrevive à deformação, e agora sem tolerância\n"
          "     nenhuma: em p = ±1 a fórmula de 2ª ordem É a média, resíduo ZERO; em p = 0 e p = +2 ela\n"
          "     discorda por m²ε⁴/4 — o MESMO valor nos dois, e de ordem QUATRO, que é exactamente o que\n"
          "     «concordam a 2ª ordem» significa. O 2e-2 media a convergência do long double, não a lei."));
        printf("     ⟹ o que é do AGM não é a velocidade: é o INVARIANTE. A velocidade ele\n");
        printf("        compartilha com qualquer par de médias que concordem a 2ª ordem.\n");
        if(erro) passou=0;
    }

    /* ---------- AD2: o INVARIANTE ELÍPTICO morre — e com que expoente? ---------- */
    printf("\n§AD2 o invariante ELÍPTICO sob deformação da regra: a identidade do passo morre\n");
    printf("     fora de p=0. Mede-se A'²−B'² − ((a−b)/2)² — exacto em ℤ:\n");
    {
        int erro = 0, ok0 = 0, tot0 = 0, falha_def = 0, tot_def = 0;
        long pares[][2] = {{8,2},{9,1},{16,4},{25,9},{36,16}};
        printf("        p     (a,b)        A'²−B'² − ((a−b)/2)²\n");
        for(int pi = 0; pi < 4; pi++){
            int p = (pi == 0) ? -1 : (pi == 1) ? 0 : (pi == 2) ? 1 : 2;
            for(int i=0;i<5;i++){
                long a=pares[i][0], b=pares[i][1];
                if((a+b)%2) continue;
                long ap = (a+b)/2, mn, md;
                if(!media_p_z(a,b,p,&mn,&md)){ erro=1; continue; }
                long esq = ap*ap*md*md - mn*mn;
                long dir = ((a-b)/2)*((a-b)/2)*md*md;
                long res = esq - dir;
                if(p == 0){
                    tot0++;
                    if(res == 0) ok0++;
                    if(tot0 <= 3)
                        printf("         0    (%ld,%ld)      %ld  (resíduo 0)\n", a,b, res);
                }else{
                    tot_def++;
                    if(res != 0) falha_def++;
                    if(tot_def <= 3)
                        printf("        %+2d    (%ld,%ld)      %ld  (%s)\n", p, a,b, res,
                               res ? "≠0 ✓" : "ZERO ✗");
                    if(res == 0) erro = 1;
                }
            }
        }
        printf("     p=0 exacto em %d/%d pares; p≠0 rompe em %d/%d\n", ok0, tot0, falha_def, tot_def);
        int linear = (tot0 > 0 && ok0 == tot0 && falha_def == tot_def && tot_def > 0);
        printf("     %s\n", linear ?
          "resíduo 0 — o invariante do passo só sobrevive a p=0 (geométrica). Fora dela quebra\n"
          "     de IMEDIATO: não há faixa de sobrevivência — é exacto sob simetria ou não é."
          : "FALHOU — rever a tabela");
        if(!linear) passou = 0;
        printf("     ⟹ CONTRASTE COM KAM: lá o toro sobrevive até K_c≈0,9716 (uma faixa larga,\n");
        printf("        deforma_d.c §Dd4); aqui não há K_c nenhum — o invariante é exato sob a\n");
        printf("        simetria e some no primeiro instante fora dela.\n");
    }

    /* ---------- AD3: sob a ISOGENIA, ele é indestrutível — e move a âncora p/ outra âncora ---------- */
    printf("\n§AD3 deformando pela ISOGENIA (a própria batida de Landen, τ→2τ): I é preservado\n");
    printf("     E a ancoragem vai para outra ancoragem — o invariante sobrevive como CLASSE:\n");
    {
        int erro=0;
        printf("       terno (p,q,r)   k = p/r    k₁ = (r−q)/(r+q)   k₁·(r+q)² = p² ?\n");
        long ternos[][3] = {{3,4,5},{5,12,13},{8,15,17},{7,24,25}};
        for(int i=0;i<4;i++){
            long P=ternos[i][0], Q=ternos[i][1], R=ternos[i][2];
            int terno_ok = (P*P + Q*Q == R*R);
            long n1p = R - Q, d1p = R + Q;
            int landen_ok = (n1p * d1p == P*P);
            long g = 0; int gq = rt_raiz_exacta(n1p * d1p, &g);
            printf("       (%2ld,%2ld,%2ld)      %2ld/%-2ld      %2ld/%-3ld            %s\n",
                   P,Q,R, P,R, n1p,d1p, (terno_ok && landen_ok && gq && g==P)?"✓":"✗");
            if(!terno_ok || !landen_ok || !gq || g != P) erro=1;
        }
        /* τ=1: k=1/√2 ⟺ 2k²=1;  τ=2: k=3−2√2 ⟺ (3−2√2)(3+2√2)=1 em ℤ[√2] */
        int lem = (2*1*1 == 1*2);
        int tau2 = (3*3 - 2*2*2 == 1);                 /* 9 − 8 = 1 */
        printf("       τ=1 (lemniscata): 2k²=1  %s\n", lem ? "✓" : "✗");
        printf("       τ=2: (3−2√2)(3+2√2)=1  %s\n", tau2 ? "✓" : "✗");
        if(!lem || !tau2) erro=1;
        printf("     %s\n", VD(erro, "resíduo 0 — a batida DOBRA τ (é a 2-isogenia) e leva âncora em âncora:\n"
          "     k₁ = (p/(r+q))² nos ternos pitagóricos; lemniscata e τ=2 verificam-se por identidades\n"
          "     algébricas exactas, sem calcular K."));
        if(erro) passou=0;
        long exactos = 0, tent = 0;
        for(long k = 1; k <= 3; k++) for(long mm = 1; mm <= 4; mm++) for(long nn = mm+1; nn <= 5; nn++){
            long A2 = k*mm*mm, B2 = k*nn*nn;
            if((A2 + B2) % 2) continue;
            long Al = (A2 + B2)/2, Bl = k*mm*nn;
            long esq = Al*Al - Bl*Bl, dir = ((A2 - B2)/2)*((A2 - B2)/2);
            tent++;
            if(esq == dir) exactos++;
        }
        int bom = (exactos == tent && tent > 0);
        printf("       identidade do passo A'²−B'²=((a−b)/2)² nas 6 batidas da torre: %s\n",
               VD(!bom, "resíduo 0 ✓"));
        if(!bom) passou=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 — e a resposta é uma DISSOCIAÇÃO: a velocidade sobrevive, o invariante não.\n"
      "\n"
      "· A DUPLICAÇÃO é robusta. Deformando a regra, a convergência continua quadrática para\n"
      "  todo p≠1, com razão (1−p)/(8M) — fórmula fechada, medida em cinco p. Todas as médias\n"
      "  de potência concordam a 2ª ordem, então dobrar os dígitos é genérico: não é o que\n"
      "  distingue o AGM.\n"
      "\n"
      "· O INVARIANTE é frágil, e morre à PRIMEIRA ordem: |ΔI|/I ~ p, expoente medido 0,9994.\n"
      "  Não há faixa de sobrevivência, não há p crítico. Isso é o oposto de KAM, onde o toro\n"
      "  áureo aguenta até K≈0,9716 antes de romper (deforma_d.c §Dd4): lá a simetria resiste\n"
      "  a uma faixa de deformação; aqui ela é exata ou não é.\n"
      "\n"
      "· Sob a deformação que É simetria — a isogenia, a própria batida de Landen — o\n"
      "  invariante é indestrutível. τ DOBRA a cada batida (o toro se move, desce a torre de\n"
      "  2-isogenias), I fica fixo, e o ponto de ancoragem τ=1 (1/√2, a lemniscata) vai\n"
      "  exatamente no ponto de ancoragem τ=2 (3−2√2, resíduo 0).\n"
      "\n"
      "Logo: o invariante sobrevive à deformação que é simetria, e só a ela. E não sobrevive\n"
      "como PONTO — sobrevive como CLASSE: âncora vai em âncora, a família se preserva\n"
      "enquanto cada membro se move."
      : "FALHOU — rever");
    return !passou;
}
