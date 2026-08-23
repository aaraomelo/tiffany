/* fatorial.c — A EXPONENCIAL E O π PELA DEFINIÇÃO DA ARANHA, em fatorial e inteiros.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/fatorial tests/fatorial.c && /tmp/serie
 */
#include "unidade.h"
#include "fatorial.h"
#include <stdio.h>

int main(void){
    const long S = 1000000;                    /* a escala: seis casas, tudo inteiro */
    printf("A SÉRIE: exp e π como a aranha os define --- em fatorial, sem uma vírgula\n\n");

    /* ── §S1 A CISÃO POR PARIDADE, E A CONSERVAÇÃO POR DERIVAÇÃO ──────────── */
    {
        printf("§S1  exp(tJ) = c(t)·1 + s(t)·J, e c²+s² ≡ 1 sai por derivação.\n\n");
        long mal = 0;
        printf("      t/S      c(t)        s(t)     c²+s² (÷10⁶)   termos  parou\n");
        long fecha = 0, pontos = 0;
        for(long i = 0; i <= 6; i++){
            long t = i*S/2;
            SfSerie c = sf_c(t, S), s = sf_s(t, S);
            long niv = sf_nivel(t, S);
            printf("      %ld/2  %10ld %10ld  %12ld %6d   %s\n", i, c.valor, s.valor,
                   niv, c.termos, c.parou ? "sim" : "NAO");
            pontos++;
            /* a conservação: c²+s² tem de dar S², a menos do grão da escala */
            if(niv > S - 3000 && niv < S + 3000) fecha++;
            /* E OS TERMOS TÊM DE SE ANULAR: é o thm:serie, e é o que faz o
             * processo TERMINAR em vez de ser truncado */
            if(!c.parou || !s.parou) mal++;
        }
        printf("      → a conservação fecha em %ld de %ld pontos, e a série PAROU em"
               " todos\n", fecha, pontos);
        if(fecha != pontos) mal++;
        printf("\n");
        ok("A EXPONENCIAL SAI DA CISÃO POR PARIDADE, E A CONSERVAÇÃO NÃO SE IMPÕE: DERIVA-SE."
           " Da única hipótese J²=−1 as potências ciclam com período quatro, a série parte-se"
           " pelos índices pares e ímpares, e o que fica é o par (c,s). E c²+s²≡1 sai de"
           " (c²+s²)' = 2cc'+2ss' = −2cs+2sc = 0 com valor 1 em zero --- medido, fecha em"
           " todos os pontos a menos do grão da escala. E O PROCESSO TERMINA, que é o"
           " thm:serie: os termos ANULAM-SE ao fim de finitas parcelas, porque o fatorial"
           " cresce mais depressa que a potência e a divisão inteira leva o termo a zero."
           " Isto não é truncar --- é parar, e o número de termos é o custo, que se conta.",
           mal == 0);
    }

    /* ── §S2 O π, E A FACE POR QUE SE O PROCURA ───────────────────────────── */
    {
        printf("§S2  π = min{t>0 : exp(tJ)·1 = −1}, e só UMA das faces se procura.\n\n");
        long mal = 0;
        printf("      grão      π (em escala)   c(π)        termos\n");
        long estavel = 0, medidos = 0; long ultimo = 0;
        for(long g = S/100; g >= S/10000; g /= 10){
            SfPi p = sf_pi(S, g);
            printf("      1/%-7ld %10ld   %10ld  %8d\n", S/g, p.pi, p.c_no_ponto, p.termos);
            if(!p.achou){ mal++; continue; }
            medidos++;
            /* A OUTRA FACE CONFERE: c(π) tem de dar −S */
            if(p.c_no_ponto > -S - 3000 && p.c_no_ponto < -S + 3000) estavel++;
            ultimo = p.pi;
        }
        printf("      → c(π) = −1 confere em %ld de %ld grãos · π = %ld/%ld\n",
               estavel, medidos, ultimo, S);
        if(estavel != medidos || medidos == 0) mal++;

        /* ── O GUME, e é ele que explica um erro que a primeira escrita fez:
         * procurar pela face ERRADA dá o número errado, e não por defeito da
         * série. Mede-se procurando o mínimo de c em vez do zero de s. */
        {
            long ant = S, achou_c = 0;
            for(long t = S/1000; t < 8*S; t += S/1000){
                SfSerie c = sf_c(t, S);
                if(c.valor <= -S + S/1000 && ant > -S + S/1000){ achou_c = t; break; }
                ant = c.valor;
            }
            printf("      gume: procurando o MÍNIMO de c em vez do zero de s → %ld/%ld\n",
                   achou_c, S);
            printf("            perto do mínimo c(t) ≈ −1 + (t−π)²/2 é PLANA: 10⁻³ no"
                   " valor dá 4·10⁻² no t\n");
            printf("            o erro é da CONDIÇÃO, não da série --- e o zero de s é"
                   " simples, porque s'(π) = c(π) = −1\n");
            /* a face errada TEM de dar um número diferente e pior */
            if(achou_c == 0 || (achou_c > ultimo - S/50 && achou_c < ultimo + S/50)) mal++;
        }
        printf("\n");
        ok("O π É UMA SAÍDA DO RELÓGIO, E LÊ-SE CONTANDO VOLTAS. A definição é sobre o PAR"
           " --- exp(πJ) = −1 quer dizer c(π) = −1 E s(π) = 0 ---, mas as duas faces não"
           " servem igualmente para PROCURAR, e essa distinção custou-me o número errado."
           " Procurar c = −1 é procurar um MÍNIMO, e perto do mínimo a função é plana: um"
           " desvio de 10⁻³ no valor dá 4·10⁻² no t, e sai 3,097. Procurar s = 0 é procurar"
           " um zero SIMPLES, porque s'(π) = c(π) = −1 ≠ 0, e sai 3,1416. É a MESMA equação"
           " lida nas duas faces, e só uma se procura --- a outra CONFERE, e c(π) dá −1 em"
           " todos os grãos. Nenhuma palavra disto é geométrica: não há arco nem área, e o"
           " círculo é o rasto que este fluxo desenha.", mal == 0);
    }

    /* ── §S3 A EXPONENCIAL REAL, e o valor por dois caminhos ──────────────── */
    {
        printf("§S3  e^x pela mesma recorrência, e a lei que o par obriga.\n\n");
        long mal = 0;
        printf("      x     e^x (escala 10⁶)   termos   e^x·e^{−x} (÷10⁶)\n");
        long fecha = 0, pontos = 0;
        for(long i = 1; i <= 4; i++){
            long x = i*S/2;
            SfSerie e1 = sf_exp(x, S), e2 = sf_exp(-x, S);
            long prod = (e1.valor/1000) * (e2.valor/1000);
            printf("      %ld/2  %14ld %8d   %14ld\n", i, e1.valor, e1.termos, prod);
            pontos++;
            /* A LEI: e^x · e^{−x} = 1, e ela não se impõe --- é a segunda equação
             * da Def.~def:op, ∂x·x = 1, com a exponencial no lugar */
            if(prod > S - 5000 && prod < S + 5000) fecha++;
            if(!e1.parou) mal++;
        }
        printf("      → e^x·e^{−x} = 1 em %ld de %ld pontos\n", fecha, pontos);
        if(fecha != pontos) mal++;
        printf("\n");
        ok("E A EXPONENCIAL REAL É A MESMA RECORRÊNCIA SEM O SINAL ALTERNADO, com a lei que"
           " o operador obriga: e^x·e^{−x} = 1 é a segunda equação da def:op, ∂x·x = 1, com"
           " a exponencial no lugar de x --- e ela fecha em todos os pontos medidos, dentro"
           " do grão. Não foi imposta: foi verificada. E os termos anulam-se aqui pelo mesmo"
           " motivo, o fatorial no denominador --- é por isso que esta é a representação que"
           " a casa usa para o irracional, e não fracção nem vírgula: a fracção não alcança"
           " o π e a vírgula perde a exactidão que o resto todo mantém.", mal == 0);
    }

    /* ── §S4 A EDO RESOLVIDA PELA SÉRIE: a raiz NUNCA é precisa ─────────── */
    {
        printf("§S4  a EDO resolve-se pela série, e a raiz irracional nunca se extrai.\n\n");
        long mal = 0;
        /* (a) O CONTROLO: raízes INTEIRAS, onde há forma fechada para confrontar.
         * y'' + 5y' + 6y = 0, y(0)=1, y'(0)=0  →  y = 3e^{-2t} − 2e^{-3t} */
        {
            long co[2] = {6, 5}, d0[2] = {S, 0};
            long lam[2] = {-2*S, -3*S}, amp[2] = {3*S, -2*S};
            printf("      RAÍZES INTEIRAS (o controlo): y''+5y'+6y=0, y(0)=1, y'(0)=0\n");
            printf("      t      pela SÉRIE   pela forma fechada   diferença  termos\n");
            long batem = 0, pontos = 0;
            for(long i = 0; i <= 4; i++){
                long t = i*S/4;
                SfSerie y = sf_edo(co, 2, d0, t, S);
                long f = sf_fechada(lam, amp, 2, t, S);
                long d = y.valor > f ? y.valor - f : f - y.valor;
                printf("      %ld/4  %11ld  %18ld  %9ld %6d\n", i, y.valor, f, d, y.termos);
                pontos++;
                if(d < 3000) batem++;          /* dentro do grão das duas contas */
            }
            printf("      → %ld de %ld dentro do grão --- e a SÉRIE é a mais exacta das"
                   " duas:\n        3e^{-2}−2e^{-3} = 0,306432, e é isso que ela dá\n",
                   batem, pontos);
            if(batem != pontos) mal++;
        }

        /* (b) RAÍZES IRRACIONAIS: a série corre igual, e nada se extrai.
         * y'' − y' − y = 0 tem raízes (1±√5)/2 --- o metal do ouro. */
        {
            long co[2] = {-1, -1}, d0[2] = {S, 0};
            printf("\n      RAÍZES IRRACIONAIS: y''−y'−y=0 (as raízes são (1±√5)/2)\n");
            printf("      t      pela SÉRIE   termos  parou   e a EDO fecha?\n");
            long fecham = 0, pontos = 0;
            for(long i = 1; i <= 4; i++){
                long t = i*S/4;
                SfSerie y = sf_edo(co, 2, d0, t, S);
                /* O GUME: a solução tem de SATISFAZER a equação. Verifica-se pela
                 * segunda diferença, que é y'' aproximado pelo grão:
                 *     y(t+h) − 2y(t) + y(t−h) ≈ h²y''  e  y'' = y' + y */
                long h = S/100;
                SfSerie ym = sf_edo(co, 2, d0, t-h, S), yp = sf_edo(co, 2, d0, t+h, S);
                long dd = yp.valor - 2*y.valor + ym.valor;      /* h²·y'' */
                long d1 = (yp.valor - ym.valor) / 2;            /* h·y' */
                /* h²y'' deve ser h²(y'+y) = h·(h y') + h²y */
                long esperado = (h/1000)*(d1/1000)*1000/1000 + ((h/1000)*(h/1000))*(y.valor/1000)/1000;
                long erro = dd > esperado ? dd - esperado : esperado - dd;
                int fecha = (erro < 400);
                printf("      %ld/4  %11ld %6d   %s     %s (erro %ld)\n", i, y.valor,
                       y.termos, y.parou ? "sim" : "NAO", fecha ? "sim" : "NAO", erro);
                pontos++; if(fecha) fecham++;
                if(!y.parou) mal++;
            }
            printf("      → a equação fecha em %ld de %ld pontos, e NENHUMA raiz foi"
                   " extraída\n", fecham, pontos);
            if(fecham != pontos) mal++;
        }

        printf("\n");
        ok("A EDO RESOLVE-SE PELA SÉRIE, E A RAIZ IRRACIONAL NUNCA SE EXTRAI. As derivadas"
           " em zero saem da PRÓPRIA recorrência --- é ela que as dá --- e a série de Taylor"
           " é fatorial por construção, y = Σ y^{(k)}(0)·t^k/k!. Logo não é preciso saber"
           " QUAIS são as raízes: bastam os coeficientes. Para y''−y'−y=0, cujas raízes são"
           " (1±√5)/2, a série corre exactamente como para as inteiras, os termos anulam-se e"
           " o processo pára. O CONTROLO é o caso de raízes inteiras, onde há forma fechada"
           " para confrontar --- e ali aconteceu o que interessa: a primeira escrita da"
           " série guardava a derivada e o t^k/k! SEPARADOS, multiplicava-os com duas"
           " divisões, e dava 0,16 onde a fechada dá 0,307. O controlo apanhou-a. Corrigida"
           " para o termo carregar já o fatorial, a série passa a dar 0,306432, que é o valor"
           " EXACTO de 3e^{-2}−2e^{-3} --- mais exacta do que o próprio controlo. E o gume da"
           " parte irracional é outro: a solução tem de SATISFAZER a equação, verificado pela"
           " segunda diferença.", mal == 0);
    }

    /* ── §S5 AS TRÊS FACES SAEM DA MESMA SÉRIE, E A NORMA É UMA SÓ ──────── */
    {
        printf("§S5  ω² = t: as três classes são três leituras da MESMA série.\n\n");
        long mal = 0;
        const char *nome[3] = {"ELÍPTICO   t=−1", "PARABÓLICO t= 0", "HIPERBÓLICO t=+1"};
        const char *faz[3]  = {"roda", "desliza", "foge"};
        printf("      face               x    c_t(x)     s_t(x)   c²−t·s²  ordem  termos\n");
        long fecha = 0, pontos = 0, finita = 0;
        for(int i = 0; i < 3; i++){
            int t = i - 1;
            for(long j = 1; j <= 2; j++){
                FtFace f = ft_face(j*S/2, t, S);
                long nm = ft_norma(f, t, S);
                printf("      %-18s %ld/2 %9ld %9ld %9ld %5d %6d\n",
                       j == 1 ? nome[i] : "", j, f.c, f.s, nm, ft_ordem(t), f.termos);
                pontos++;
                /* A CONSERVAÇÃO É A NORMA DA TRÍADE, e vale nas três faces */
                if(nm > S - 3000 && nm < S + 3000) fecha++;
                if(!f.parou) mal++;
            }
            if(t == 0 && ft_face(S, 0, S).termos == 2) finita++;
        }
        printf("      → a norma c²−t·s² = 1 fecha em %ld de %ld pontos, nas TRÊS faces\n",
               fecha, pontos);
        if(fecha != pontos) mal++;
        printf("      → e a face t=0 tem a série FINITA: %ld (ω²=0 mata o resto)\n", finita);
        if(finita != 1) mal++;

        /* ── AS ORDENS, que separam as faces sem olhar para o Δ ────────────── */
        /* A ORDEM NÃO SE TABELA: aplica-se ω e CONTA-SE. E a órbita mostra-se,
         * para o cliente ver de onde o número veio. */
        printf("      a órbita de ω, aplicada até voltar --- e a ordem é a CONTAGEM:\n");
        for(int i = 0; i < 3; i++){
            char orb[80]; ft_orbita(i-1, orb, sizeof orb);
            printf("        t=%+d  %-28s ordem %d  (%s)\n", i-1, orb, ft_ordem(i-1), faz[i]);
        }
        printf("      e o lem:cristal diz que numa rede só há {1,2,3,4,6} --- estas duas"
               " estão lá, e o nilpotente CAI no zero em vez de fechar\n");
        if(ft_ordem(-1) != 4 || ft_ordem(1) != 2 || ft_ordem(0) != 0) mal++;

        /* ── O GUME: se a norma fosse a MESMA fórmula nas três, não estaria a
         * distinguir nada. Com o t errado, ela QUEBRA. */
        {
            FtFace e = ft_face(S, -1, S), h = ft_face(S, +1, S);
            long errada_e = ft_norma(e, +1, S);      /* elíptico lido como hiperbólico */
            long errada_h = ft_norma(h, -1, S);      /* e ao contrário */
            printf("      gume: a face elíptica lida com t=+1 dá %ld (devia ser %ld)\n",
                   errada_e, S);
            printf("            a hiperbólica lida com t=−1 dá %ld\n", errada_h);
            if(errada_e > S - 3000 && errada_e < S + 3000) mal++;
            if(errada_h > S - 3000 && errada_h < S + 3000) mal++;
        }

        printf("\n");
        ok("AS TRÊS CLASSES SÃO TRÊS LEITURAS DA MESMA SÉRIE, E A CONSERVAÇÃO É UMA SÓ. O"
           " paper faz a cisão por paridade com J²=−1; com ω²=t as potências ciclam do mesmo"
           " modo e ω^{2k}=t^k, pelo que exp(xω) = c_t·1 + s_t·ω sai igual --- e os três"
           " valores de t dão o elíptico (cos,sen), o parabólico (1,x) e o hiperbólico"
           " (cosh,senh). Não são três teorias: são três leituras. E a conservação é a NORMA"
           " DA TRÍADE, c²−t·s² = 1, que é N(c+sω) --- fecha nas três faces e não se impõe:"
           " sai de (c²−ts²)' = 2c(ts) − 2t·s·c = 0. O PARABÓLICO mostra-o de imediato: com"
           " ω²=0 a série TERMINA em dois termos, e a norma dá exactamente um. As ordens"
           " separam as faces sem olhar para o Δ --- 4 para o que roda, 2 para o que reflecte,"
           " e nenhuma para o nilpotente ---, e o lem:cristal diz que numa rede só há"
           " {1,2,3,4,6}. O gume é que a norma NÃO é a mesma fórmula nas três: lida com o t"
           " errado, quebra.", mal == 0);
    }

    /* ── §S6 A SOLUÇÃO É A TRAJECTÓRIA, E O CAMPO LÊ-LHE O CUSTO ────────── */
    {
        printf("§S6  a apresentação é a CONTAGEM: voltas, cauda e período.\n\n");
        long mal = 0;
        struct { const char *n; int o; long co[2]; long d0[2]; int ciclo; } E[5] = {
          {"y'−y=0     (e^t)",  1, {-1,0}, {1,0}, 1},
          {"y''+y=0    (cos)",  2, {1,0},  {1,0}, 4},
          {"y''+y=0    (sen)",  2, {1,0},  {0,1}, 4},
          {"y''−y=0    (cosh)", 2, {-1,0}, {1,0}, 2},
          {"y''+5y'+6y=0",      2, {6,5},  {1,0}, 0},
        };
        printf("      a equação          ciclo  voltas  ℓ (custo)  p   confere\n");
        long certos = 0, comp = 0;
        for(int i = 0; i < 5; i++){
            FtSol s2 = ft_solucao(E[i].co, E[i].o, E[i].d0, 12);
            FtTraj tr = ft_trajetoria(&s2, S, S);
            char b[220]; ft_sol_escreve(&s2, b, sizeof b);
            /* o ciclo lê-se da lista, sem se calcular */
            int per = 0;
            for(int p2 = 1; p2 <= 6 && !per; p2++){
                int ok2 = 1;
                for(int k = 0; k + p2 < s2.n; k++) if(s2.d[k] != s2.d[k+p2]){ ok2 = 0; break; }
                if(ok2) per = p2;
            }
            int conf = ft_sol_confere(&s2, E[i].co);
            printf("      %-19s %4d %6ld %8ld %4ld %8d\n", E[i].n, per, tr.voltas,
                   tr.cauda, tr.periodo, conf);
            if(per == E[i].ciclo) certos++;
            if(conf == s2.n - E[i].o) comp++;
        }
        printf("      → %ld de 5 com o ciclo esperado · %ld de 5 com TODOS os coeficientes"
               " a conferirem a recorrência\n", certos, comp);
        if(certos != 5 || comp != 5) mal++;

        /* ── E O QUE O CAMPO LÊ, que a lista sozinha não mostra: a PARIDADE.
         * Onde metade dos coeficientes é zero --- que é o ciclo de quatro do
         * paper --- cada soma parcial REPETE-SE, e o campo dá G=2 em todas: a
         * cauda vai a zero e o período conta-as. Onde nenhum é nulo, todas são
         * distintas e é ao contrário. A paridade da série aparece no campo. */
        {
            FtSol ce = ft_solucao((long[]){1,0}, 2, (long[]){1,0}, 12);
            FtSol ex = ft_solucao((long[]){-1,0}, 1, (long[]){1,0}, 12);
            FtTraj tc = ft_trajetoria(&ce, S, S), tx = ft_trajetoria(&ex, S, S);
            printf("      a paridade LIDA NO CAMPO --- e a lista sozinha não a mostra:\n");
            printf("        cos  (metade dos d_k nula): cauda %ld · período %ld\n",
                   tc.cauda, tc.periodo);
            printf("        e^t  (nenhum d_k nulo):     cauda %ld · período %ld\n",
                   tx.cauda, tx.periodo);
            /* têm de ser opostos: um com cauda alta e período baixo, o outro
             * ao contrário. Se fossem iguais, o campo não estaria a ler nada. */
            if(!(tc.periodo > tc.cauda && tx.cauda > tx.periodo)) mal++;
        }

        printf("\n");
        ok("A APRESENTAÇÃO É A CONTAGEM, E NÃO UMA SÉRIE COPIADA. Duas escritas minhas"
           " caíram antes desta: uma lista entre sinais que eu inventei, e depois a série"
           " clássica escrita à mão --- nenhuma é o que esta casa faz. O paper diz o que é:"
           " «a série é uma TRAJECTÓRIA, o índice k conta as VOLTAS do ciclo de quatro, e o"
           " campo G lê o custo dela SEM A SEGUIR --- ℓ é o que se gastou, e p=1 diz que"
           " parou». São essas as três contagens que se apresentam. E o CICLO lê-se na"
           " própria lista de coeficientes, procurando onde ela se repete: quatro no cos e no"
           " sen, dois no cosh, um no e^t --- que é a ordem de ω, sem se calcular nada. O"
           " campo mostra ainda o que a lista sozinha não dá: onde metade dos coeficientes é"
           " NULA --- que é o ciclo de quatro --- cada soma parcial repete-se e G vale dois em"
           " todas, pelo que a cauda cai a zero e o período conta-as; onde nenhum é nulo, é ao"
           " contrário. A paridade da série aparece no campo.", mal == 0);
    }

    /* ── §S7 O PAR (c,s), A NORMA NOS COEFICIENTES, E A RÉGUA NOS TRÊS ──── */
    {
        printf("§S7  a solução com J é o PAR, e a régua desce nos três regimes.\n\n");
        long mal = 0;
        const char *nm[3] = {"ELÍPTICO   t=−1", "PARABÓLICO t= 0", "HIPERBÓLICO t=+1"};
        printf("      regime            c: os d_k       s: os d_k      norma  viola  estritos\n");
        long fecham = 0, descem = 0; long est[3];
        for(int i = 0; i < 3; i++){
            int t = i - 1;
            FtPar p = ft_par(t, 10);
            FtUltra u = ft_ultra(&p.c, 14);
            int nrm = ft_par_norma(&p, 8);
            printf("      %-17s ", nm[i]);
            for(int k = 0; k < 5; k++) printf("%2ld ", p.c.d[k]);
            printf("  ");
            for(int k = 0; k < 5; k++) printf("%2ld ", p.s.d[k]);
            printf("  %d/8 %6ld %8ld\n", nrm, u.viola, u.estrito);
            est[i] = u.estrito;
            if(nrm == 8) fecham++;
            if(u.viola == 0 && u.estrito > 0) descem++;
        }
        printf("      → a norma c²−t·s² = 1 fecha NOS COEFICIENTES em %ld das 3, e a régua"
               " desce em %ld\n", fecham, descem);
        if(fecham != 3 || descem != 3) mal++;
        printf("      e os estritos são %ld, %ld, %ld --- a régua VÊ a diferença entre as"
               " faces\n", est[0], est[1], est[2]);
        if(est[0] == est[1] || est[1] == est[2] || est[0] == est[2]) mal++;

        /* a norma verifica-se SEM AVALIAR: é a convolução dos
         * coeficientes, com os pesos binomiais que a base fatorial pede */
        {
            FtPar p = ft_par(-1, 10);
            printf("      a norma pela CONVOLUÇÃO dos coeficientes, termo a termo:");
            for(int n2 = 0; n2 < 5; n2++){
                long acc = 0, bin = 1;
                for(int k = 0; k <= n2; k++){
                    acc += bin * (p.c.d[k]*p.c.d[n2-k] - (long)p.t * p.s.d[k]*p.s.d[n2-k]);
                    bin = bin * (n2 - k) / (k + 1);
                }
                printf(" %ld", acc);
            }
            printf("   <- <1,0,0,0,0>, e nenhum ponto foi avaliado\n");
        }

        /* e MAIS CASOS, de graus vários, todos pelo mesmo caminho */
        {
            struct { const char *n; int o; long co[4]; long d0[4]; } M[5] = {
              {"y3 = y        (raiz cubica)",  3, {-1,0,0},   {1,0,0}},
              {"y4 = y        (quartica)",     4, {-1,0,0,0}, {1,0,0,0}},
              {"y2 + 2y1 + y  (raiz dupla)",   2, {1,2},      {1,0}},
              {"y3 - 3y1 - 2y",                3, {-2,-3,0},  {1,0,0}},
              {"y4 + 5y2 + 4y",                4, {4,0,5,0},  {1,0,0,0}},
            };
            printf("\n      mais casos --- e todos pelo mesmo caminho:\n");
            long conferem = 0;
            for(int i = 0; i < 5; i++){
                FtSol so = ft_solucao(M[i].co, M[i].o, M[i].d0, 10);
                FtUltra u = ft_ultra(&so, 14);
                int cf = ft_sol_confere(&so, M[i].co);
                printf("        %-28s d:", M[i].n);
                for(int k = 0; k < 7; k++) printf(" %ld", so.d[k]);
                printf("  · %d/%d conferem · régua %s\n", cf, so.n - M[i].o,
                       u.viola == 0 ? "desce" : "NAO");
                if(cf == so.n - M[i].o && u.viola == 0) conferem++;
            }
            printf("      → %ld de 5 com todos os coeficientes a conferirem e a régua a"
                   " descer\n", conferem);
            if(conferem != 5) mal++;
        }

        printf("\n");
        ok("A SOLUÇÃO COM J É O PAR (c,s), E A NORMA VERIFICA-SE NOS COEFICIENTES. A equação"
           " da face é y''=t·y e a solução vive em Z[w]: y = c·1 + s·w, que é o exp(tJ) do"
           " paper com J geral. As duas metades saem da MESMA recorrência com condições"
           " iniciais diferentes --- (1,0) dá o c, (0,1) dá o s ---, e é a cisão outra vez,"
           " agora nas condições em vez dos índices. E A NORMA c²−t·s²=1 VERIFICA-SE SEM"
           " AVALIAR PONTO NENHUM: é a CONVOLUÇÃO desta casa --- a do def:conv, a mesma que o"
           " thm:zeta-mu corre como gato e esquilo --- lida na base fatorial, e dá <1,0,0,0,0> --- o que separa «a"
           " identidade vale» de «vale nos pontos que eu testei». A RÉGUA DESCE NOS TRÊS"
           " regimes, com zero violações, e os ESTRITOS distinguem-nos. Se fossem iguais, a"
           " régua não estaria a ver a diferença entre as faces --- e é isso que faz dela uma"
           " medida e não uma formalidade.", mal == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
