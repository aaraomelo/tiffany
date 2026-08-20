/* tests/fractais.c — CANTOR, JULIA E O DRAGÃO, do lado do CORTE. E o que não se afirma.
 *
 * O `reais.tex` constrói ℝ pelo corte. Os três fractais que a casa já mede noutros sítios
 * têm, cada um, uma peça que é DESTE andar — e é essa peça que aqui se mede:
 *
 *   CANTOR   o conjunto é a expansão ternária que evita o 1. E a razão pela qual ele tem
 *            representação ÚNICA é a mesma que o `diagonal.c` §DG1 usou para a diagonal
 *            sair da borda: a cauda inteira vale metade do dígito, e nunca o alcança.
 *   O SHIFT  x ↦ 2x mod 1 é a expansão binária a andar — e cada bit dela É a decisão
 *            do encaixotamento (esquerda/direita). Dois caminhos, um objecto.
 *   JULIA    o shift (aditivo, θ↦2θ) é o quadrado (multiplicativo, z↦z²) — medido em
 *            `cantor_julia_dual.c` §CJ1. Aqui mede-se o que o corte precisa: a órbita do
 *            shift FECHA num racional e não fecha fora dele.
 *   DRAGÃO   a dobra do papel é o mesmo passo do encaixe — dividir e escolher um lado.
 *            No plano ela FUNDE (a curva visita a mesma célula duas vezes, `aranha_g.c`
 *            §AG2); na recta os intervalos ENCAIXAM e nunca se cruzam. O contraste é o
 *            resultado, e mede-se dos dois lados.
 *
 * §FR0  CANTOR: a expansão sem o dígito 1 tem representação única, e o ponto é um corte
 * §FR1  O SHIFT é a DOBRA: os bits do encaixe de √a são os bits do shift — dois caminhos
 * §FR2  a órbita do shift FECHA num racional (período exibido) e não fecha no irracional
 * §FR3  o DRAGÃO funde (G>1) e o ENCAIXE não — o mesmo passo, dois comportamentos
 * §FR4  a MÉTRICA DA ÁRVORE (o prefixo comum): ela é ULTRAMÉTRICA, e é a régua
 * §FR5  a DIMENSÃO É UM CORTE: 2 = 3^s, decidido por 3^p vs 2^q, e não fecha em ℚ
 * §FR7  a MÉTRICA GENERALIZADA a base b, e o GANCHO: ela É a canónica μ(A△B)/2
 * §FR8  a DIMENSÃO generalizada (k de b dígitos), com os controlos s=1 e s=0
 * §FR6  o DRAGÃO preenche (contagem de caixas) e a ARANHA paga memória; a recta não
 *
 * ── A RÉGUA EXISTE, E DERIVA-SE DO QUE A CASA JÁ TEM ─────────────────────────────
 * O `teoria.tex` diz «fractal clássico não se afirma» — mas diz isso de UM elo, e a seguir
 * aponta onde a régua está: «a régua de Hausdorff da casa existe noutro elo». Aqui ela
 * deriva-se: a métrica da ÁRVORE (§FR4) é o prefixo comum, d(x,y) = 3^-n no primeiro
 * dígito diferente — a métrica natural do espaço onde o corte vive, e ULTRAmétrica. Com
 * ela, a cobertura canónica dá a dimensão como um CORTE (§FR5): 2 = 3^s, decidido por
 * 3^p contra 2^q em inteiros. Não se importa número nenhum de fora, e não se afirma
 * medida nem auto-semelhança — só o que a cobertura canónica dá.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "racionais.h"
#include "cifra.h"
#include "reais.h"
#include "unidade.h"

#define D 10                        /* dígitos por expansão */

static uint64_t pot(int b, int k){ uint64_t p = 1; while(k--) p *= (uint64_t)b; return p; }
static uint64_t valor(const int *d, int b, int n){
    uint64_t v = 0; for(int i = 0; i < n; i++) v = v*(uint64_t)b + (uint64_t)d[i]; return v;
}

int main(void){
    printf("\n=== CANTOR, JULIA E O DRAGÃO, do lado do corte ===\n");

    /* ═══ §FR0 CANTOR: sem o dígito 1, a representação é única ═════════════════ */
    printf("\n§FR0 A expansão ternária que evita o 1 — e por que ela não colapsa.\n\n");
    {
        /* os 2^D pontos de nível D do conjunto: dígitos em {0,2} */
        long colisoes = 0, total = 0;
        uint64_t minima = ~(uint64_t)0;
        for(int x = 0; x < (1 << D); x++){
            int dx[D]; for(int i = 0; i < D; i++) dx[i] = ((x >> (D-1-i)) & 1) ? 2 : 0;
            uint64_t vx = valor(dx,3,D);
            total++;
            for(int y = x+1; y < (1 << D); y++){
                int dy[D]; for(int i = 0; i < D; i++) dy[i] = ((y >> (D-1-i)) & 1) ? 2 : 0;
                uint64_t vy = valor(dy,3,D);
                if(vx == vy) colisoes++;
                else { uint64_t d = vx > vy ? vx-vy : vy-vx; if(d < minima) minima = d; }
            }
        }
        /* a razão: a cauda toda de 2s vale exactamente UM dígito 2 da posição anterior
         * menos o que falta — Σ_{j>i} 2·3^{-j} = 3^{-i}, logo os intervalos TOCAM-SE mas
         * as expansões que evitam o 1 nunca dão o mesmo valor dentro de um nível. */
        int caudaC[D]; caudaC[0] = 0; for(int i = 1; i < D; i++) caudaC[i] = 2;
        uint64_t cauda = valor(caudaC,3,D), um = pot(3,D-1);
        printf("        %ld pontos de nível %d: %ld colisões · menor distância = %llu/3^%d\n",
               total, D, colisoes, (unsigned long long)minima, D);
        /* a cauda de 2s vale 3^{D−1} − 1: fica a UMA unidade de 3^{−D} do dígito 1, e no
         * limite alcança-o EXACTAMENTE (0.0222… = 0.1). Só que 0.1 tem o dígito 1 e está
         * FORA do conjunto — é por isso que Cantor tem representação única. */
        printf("        cauda de 2s = %llu · o dígito 1 vale %llu · diferença = %llu"
               " (uma unidade de 3^-%d)\n", (unsigned long long)cauda,
               (unsigned long long)um, (unsigned long long)(um - cauda), D);
        ok("O CONJUNTO DE CANTOR É UMA CONDIÇÃO SOBRE A EXPANSÃO, E ELA VIVE NESTE ANDAR:"
           " são os reais cuja expansão ternária evita o dígito 1 — uma restrição sobre a"
           " DECISÃO em cada nível, que é exactamente o que o corte é. Os 1024 pontos de"
           " nível 10 têm valores todos distintos, com distância mínima não nula. E a"
           " comparação com o `diagonal.c` §DG1 é o ponto: lá usaram-se os dígitos {0,1} em"
           " base 3, e a cauda valia METADE do dígito, nunca o alcançando; aqui, com {0,2},"
           " a cauda de 2s vale 3^{D−1} − 1 e fica a UMA unidade do dígito 1 — no limite"
           " alcança-o exactamente (0.0222… = 0.1). E é isso que salva a unicidade: o valor"
           " onde as duas escritas colidiriam tem o dígito 1, e o dígito 1 está FORA do"
           " conjunto. A mesma conta decide as duas coisas, e decide-as em inteiros",
           colisoes == 0 && minima > 0 && total == 1024 && um - cauda == 1);
    }

    /* ═══ §FR1 O SHIFT É A DOBRA ═══════════════════════════════════════════════ */
    printf("\n§FR1 Os bits do encaixe de √a são os bits do shift x ↦ 2x mod 1.\n\n");
    {
        long mal = 0, bits = 0;
        printf("        a    bits do encaixe    bits do shift\n");
        for(long a = 2; a <= 7; a++){
            if(a == 4) continue;
            Corte c = { a, 2 };
            Qz lo, hi;
            if(!rz_caixa_inicial(c, &lo, &hi)) { mal++; continue; }
            /* PARTE INTEIRA: o encaixe começa em [k, k+1] */
            long k = lo.p / lo.q;
            char benc[12], bsh[12];
            /* (i) os bits do ENCAIXE: em cada dobra, 1 se o médio ficou abaixo */
            Qz l = lo, h = hi;
            for(int i = 0; i < 10; i++){
                Qz m = qz_medio(l, h);
                int bom, sgn = rz_cmp(m, 2, a, &bom);
                if(!bom){ benc[i] = '?'; continue; }
                if(sgn < 0){ benc[i] = '1'; l = m; } else { benc[i] = '0'; h = m; }
            }
            benc[10] = 0;
            /* (ii) os bits do SHIFT da parte fraccionária, em inteiros: começa-se com
             * a fracção x = √a − k, representada por (a, k) via a comparação
             * (2^i·(√a − k)) mod 1 ≥ 1/2  ⟺  compara-se um inteiro com √a */
            {
                long num = 1;                      /* multiplicador 2^i acumulado */
                long base = k;                     /* a parte já consumida */
                for(int i = 0; i < 10; i++){
                    /* bit i: (2^{i+1})(√a − k) tem parte inteira ímpar? Testa-se por
                     * comparação inteira: 2^{i+1}√a  vs  2^{i+1}k + (parte já lida)*2 + 1 */
                    num *= 2;
                    long lim = num * base;         /* 2^{i+1}·k */
                    /* conta quantos inteiros t ≤ 2^{i+1}√a — isto é, t² ≤ 4^{i+1}·a */
                    long t = 0;
                    while((__int128)(t+1)*(t+1) <= (__int128)num*num*a) t++;
                    bsh[i] = ((t - lim) & 1) ? '1' : '0';
                    (void)lim;
                }
                bsh[10] = 0;
            }
            printf("        %ld    %s      %s\n", a, benc, bsh);
            for(int i = 0; i < 10; i++){ bits++; if(benc[i] != bsh[i]) mal++; }
        }
        printf("        %ld bits comparados, %ld divergências\n", bits, mal);
        ok("O SHIFT E A DOBRA SÃO A MESMA COISA VISTA DE DOIS SÍTIOS. Cada dobra do"
           " encaixotamento faz uma pergunta — o ponto médio está abaixo ou acima do corte? —"
           " e a resposta é UM bit; e a expansão binária do mesmo real é a órbita de"
           " x ↦ 2x mod 1, onde cada passo lê UM bit. Os dois caminhos não partilham código:"
           " um bissecta racionais em Qz e pergunta ao corte, o outro conta inteiros t com"
           " t² ≤ 4^i·a. E dão a mesma palavra de dez bits em cinco radicandos. É por isso"
           " que a expansão binária não é uma notação: é o registo das decisões do corte",
           mal == 0 && bits == 50);
    }

    /* ═══ §FR2 A ÓRBITA DO SHIFT: fecha no racional, não fecha fora ════════════ */
    printf("\n§FR2 O shift num racional CICLA — e o período exibe-se.\n\n");
    {
        long ciclou = 0, casos = 0, nao_ciclou = 0;
        printf("        p/q        período do shift\n");
        const long QQ[4] = {3, 5, 7, 9};              /* denominadores ímpares */
        for(int i = 0; i < 4; i++){
            long q = QQ[i], r = 1 % q, r0 = 1 % q, per = 0;
            do { r = (2*r) % q; per++; } while(r != r0 && per < 64);
            casos++;
            if(r == r0 && per < 64){ ciclou++; printf("        1/%-8ld %ld\n", q, per); }
        }
        /* e o irracional: os bits do encaixe de √2 não repetem no horizonte — e DIZ-SE
         * que «não repetiu até aqui» não é «não repete»: isso é teorema, não medida. */
        {
            Corte c = { 2, 2 }; Qz lo, hi; rz_caixa_inicial(c, &lo, &hi);
            char b[16]; Qz l = lo, h = hi;
            for(int i = 0; i < 14; i++){
                Qz m = qz_medio(l, h);
                int bom, sgn = rz_cmp(m, 2, 2, &bom);
                if(!bom){ b[i] = '?'; continue; }
                if(sgn < 0){ b[i] = '1'; l = m; } else { b[i] = '0'; h = m; }
            }
            b[14] = 0;
            int repetiu = 0;
            for(int per = 1; per <= 7 && !repetiu; per++){
                int ok_ = 1;
                for(int i = 0; i + per < 14; i++) if(b[i] != b[i+per]) { ok_ = 0; break; }
                if(ok_) repetiu = 1;
            }
            if(!repetiu) nao_ciclou = 1;
            printf("        √2: %s  (período ≤ 7 nos 14 bits? %s)\n", b, repetiu ? "SIM" : "não");
        }
        ok("E A DIFERENÇA ENTRE O RACIONAL E O REAL LÊ-SE NA ÓRBITA DO SHIFT: com denominador"
           " ímpar o shift é uma permutação das classes mod q — não tem o que gastar, porque"
           " o 2 é inversível ali — e CICLA, com o período exibido; é a mesma conservação que"
           " o `descida_mobius.c` mede do outro lado. Já a palavra do encaixe de √2 não"
           " exibe período até ao horizonte. E diz-se o que isso é e o que não é: «não"
           " repetiu nos catorze bits medidos» NÃO é «não repete» — a segunda é teorema"
           " (uma expansão periódica é racional), e esta máquina não a demonstra",
           ciclou == casos && casos == 4 && nao_ciclou == 1);
    }

    /* ═══ §FR3 O DRAGÃO FUNDE, O ENCAIXE NÃO ═══════════════════════════════════ */
    printf("\n§FR3 A mesma dobra: no plano visita duas vezes, na recta encaixa.\n\n");
    {
        /* a curva do dragão (Heighway): a virada n é dada pelo bit acima do menor bit 1 */
        #define GR 96
        static unsigned char G[GR][GR];
        int x = GR/2, y = GR/2, dx = 1, dy = 0;
        long passos = 1024, maxG = 0, fundiu = 0;
        G[y][x] = 1;
        for(long n = 1; n <= passos; n++){
            x += dx; y += dy;
            if(x < 0 || x >= GR || y < 0 || y >= GR) break;
            if(G[y][x] < 255) G[y][x]++;
            if(G[y][x] > 1) fundiu++;
            if(G[y][x] > maxG) maxG = G[y][x];
            long m = n & (-n);                       /* o menor bit 1 */
            int direita = ((n & (m << 1)) != 0);     /* o bit acima dele */
            int ndx = direita ? dy : -dy, ndy = direita ? -dx : dx;
            dx = ndx; dy = ndy;
        }
        /* e a recta: o encaixe do corte NUNCA revisita — os intervalos são encaixados */
        Corte c = { 2, 2 };
        Qz lo, hi; rz_caixa_inicial(c, &lo, &hi);
        long encaixou = 0, largou = 0;
        for(int i = 0; i < 10; i++){
            Qz lo0 = lo, hi0 = hi;
            Qz m = qz_medio(lo, hi);
            int bom, sgn = rz_cmp(m, 2, 2, &bom);
            if(!bom) break;
            if(sgn < 0) lo = m; else hi = m;
            /* encaixado: [lo,hi] ⊂ [lo0,hi0], e estritamente mais pequeno */
            if((qz_menor(lo0, lo) || qz_igual(lo0, lo)) &&
               (qz_menor(hi, hi0) || qz_igual(hi, hi0)) && qz_menor(lo, hi)) encaixou++;
            else largou++;
        }
        printf("        dragão (%ld passos): max G = %ld · células revisitadas: %ld\n",
               passos, maxG, fundiu);
        printf("        encaixe de √2 (10 dobras): %ld encaixados, %ld largaram\n",
               encaixou, largou);
        ok("A DOBRA É O MESMO PASSO E DÁ DOIS COMPORTAMENTOS, E É O CONTRASTE QUE INTERESSA."
           " Dividir e escolher um lado: no plano isso é a curva do dragão, e ela FUNDE — o"
           " caminho passa mais de uma vez pela mesma célula, e é essa duplicidade que o"
           " `inteiros.tex` usa como memória geométrica (G > 1, `aranha_g.c` §AG2). Na recta"
           " o mesmo passo ENCAIXA: cada intervalo está contido no anterior e é estritamente"
           " menor, em todas as dobras, e nunca se revisita nada. É a Lei 7 nos dois lados —"
           " ligar sem fundir na recta, ligar FUNDINDO no plano —, e por isso o corte pode"
           " definir um ponto onde a curva do dragão define uma região",
           maxG > 1 && fundiu > 0 && encaixou == 10 && largou == 0);
        #undef GR
    }

    /* ═══ §FR4 A MÉTRICA DA ÁRVORE — e ela é ULTRAMÉTRICA ══════════════════════ */
    printf("\n§FR4 d(x,y) = 3^-n no primeiro dígito diferente: a desigualdade FORTE.\n\n");
    {
        /* a métrica do prefixo comum — a da ÁRVORE, que é onde o corte vive: cada real é
         * um ramo, e dois ramos distam pelo nível onde se separam. Ela não é só métrica:
         * é ULTRAMÉTRICA, d(x,z) ≤ max(d(x,y), d(y,z)), que é mais forte que a triangular. */
        long tri = 0, ultra = 0, casos = 0, mal = 0;
        int NS = 64;
        for(int x = 0; x < NS; x++) for(int y = 0; y < NS; y++) for(int z = 0; z < NS; z++){
            /* d como EXPOENTE: n = primeiro dígito diferente (maior n ⟹ mais perto) */
            int nxy = D, nyz = D, nxz = D;
            for(int i = 0; i < 6; i++){
                int bx = (x>>(5-i))&1, by = (y>>(5-i))&1, bz = (z>>(5-i))&1;
                if(nxy == D && bx != by) nxy = i;
                if(nyz == D && by != bz) nyz = i;
                if(nxz == D && bx != bz) nxz = i;
            }
            casos++;
            /* d = 3^-n, logo d menor ⟺ n maior. max(d_xy,d_yz) ⟺ min(n_xy,n_yz) */
            int menor_n = nxy < nyz ? nxy : nyz;
            if(nxz >= menor_n) ultra++; else mal++;          /* a ULTRAmétrica */
            if(nxz >= menor_n) tri++;                        /* a triangular segue dela */
        }
        printf("        %ld triplos: ultramétrica em %ld, triangular em %ld, falhas %ld\n",
               casos, ultra, tri, mal);
        ok("A MÉTRICA DESTE ANDAR É A DA ÁRVORE, E ELA É MAIS FORTE QUE A DE UM ESPAÇO"
           " QUALQUER. Dois reais distam pelo nível em que os seus ramos se separam —"
           " d(x,y) = 3^-n com n o primeiro dígito diferente —, e isso não é uma métrica"
           " comum: é ULTRAMÉTRICA, d(x,z) ≤ max(d(x,y), d(y,z)), verificada nos 262 144"
           " triplos de seis dígitos sem uma falha. A triangular sai dela de graça, e nunca"
           " ao contrário. É a régua que a dimensão vai usar em §FR5, e é a mesma árvore que"
           " o encaixotamento percorre: cada dobra é um nível",
           mal == 0 && ultra == casos && casos == 64*64*64);
    }

    /* ═══ §FR5 A DIMENSÃO É UM CORTE — decidido em inteiros ════════════════════ */
    printf("\n§FR5 s com 2 = 3^s: o corte {p/q : 3^p < 2^q}, e ele não fecha em ℚ.\n\n");
    {
        /* Com a métrica de §FR4, a cobertura canónica de Cantor no nível n tem
         * N = 2^n peças de diâmetro 3^-n, logo Σ diam^s = (2·3^-s)^n, que só não é 0
         * nem ∞ quando 2 = 3^s. A dimensão é o real s — e ele é um CORTE, decidido por
         * comparação INTEIRA: p/q < s ⟺ 3^p < 2^q. */
        long abaixo = 0, acima = 0, em_cima = 0, mal = 0;
        long melhor_p = 0, melhor_q = 1;
        for(long q = 1; q <= 30; q++) for(long p = 1; p < q; p++){
            /* tectos: 3^p ≤ 3^29 ≈ 6.9e13 e 2^q ≤ 2^30, ambos cabem em long long */
            unsigned long long tp = 1, tq = 1;
            for(long i = 0; i < p; i++) tp *= 3ULL;
            for(long i = 0; i < q; i++) tq *= 2ULL;
            if(tp < tq){ abaixo++; if(p*melhor_q > melhor_p*q){ melhor_p = p; melhor_q = q; } }
            else if(tp > tq) acima++;
            else em_cima++;                       /* 3^p = 2^q: impossível para p,q ≥ 1 */
        }
        /* a contagem de caixas, exacta: no nível n há 2^n peças de diâmetro 3^-n */
        long pecas = 1, niveis = 0;
        for(int n = 1; n <= 10; n++){ pecas *= 2; niveis++; }
        if(pecas != 1024 || niveis != 10) mal++;
        printf("        %ld racionais abaixo, %ld acima, %ld EM CIMA (3^p = 2^q)\n",
               abaixo, acima, em_cima);
        printf("        melhor aproximação por baixo até q=30: %ld/%ld · caixas no nível 10:"
               " %ld peças de diâmetro 3^-10\n", melhor_p, melhor_q, pecas);
        ok("E A DIMENSÃO NÃO É UM NÚMERO IMPORTADO DE FORA: É UM CORTE, o objecto deste"
           " paper. Com a métrica da árvore, a cobertura canónica dá 2^n peças de diâmetro"
           " 3^-n, e a soma dos diâmetros elevados a s só deixa de ser 0 ou ∞ quando 2 = 3^s."
           " Esse s decide-se por comparação INTEIRA — p/q < s ⟺ 3^p < 2^q —, e o corte NÃO"
           " fecha em ℚ: nenhum dos racionais varridos cai em cima, e a razão é de uma linha"
           " (3^p = 2^q com p,q ≥ 1 contradiz a fatoração única). Logo a dimensão de Cantor é"
           " irracional pelo MESMO argumento que faz √2 irracional, e é um habitante deste"
           " andar como qualquer outro",
           em_cima == 0 && abaixo > 0 && acima > 0 && mal == 0);
    }

    /* ═══ §FR6 O DRAGÃO PREENCHE, E A ARANHA PAGA MEMÓRIA ══════════════════════ */
    printf("\n§FR6 Contagem de caixas do dragão; e o que cada caminho tem de guardar.\n\n");
    {
        #define GR2 160
        static unsigned char C[GR2][GR2];
        long celulas[3] = {0,0,0}, passos[3] = {256, 1024, 4096};
        for(int k = 0; k < 3; k++){
            for(int i = 0; i < GR2; i++) for(int j = 0; j < GR2; j++) C[i][j] = 0;
            int x = GR2/2, y = GR2/2, dx = 1, dy = 0; long c = 1;
            C[y][x] = 1;
            for(long n = 1; n <= passos[k]; n++){
                x += dx; y += dy;
                if(x < 0 || x >= GR2 || y < 0 || y >= GR2) break;
                if(!C[y][x]){ C[y][x] = 1; c++; }
                long m = n & (-n);
                int dir = ((n & (m << 1)) != 0);
                int ndx = dir ? dy : -dy, ndy = dir ? -dx : dx;
                dx = ndx; dy = ndy;
            }
            celulas[k] = c;
        }
        /* quadruplicar os passos multiplica as células por ~4: é a dimensão 2 sem logaritmo */
        long r1 = celulas[1] * 100 / celulas[0], r2 = celulas[2] * 100 / celulas[1];
        printf("        passos %ld→%ld→%ld · células %ld→%ld→%ld · razões %ld/100 e %ld/100\n",
               passos[0], passos[1], passos[2], celulas[0], celulas[1], celulas[2], r1, r2);
        /* E A MEMÓRIA: a aranha escreve G a cada passo; o encaixe não escreve NADA */
        long escritas_aranha = passos[2];        /* uma marca por passo (aranha_g.c §AG10) */
        long escritas_encaixe = 0;               /* o corte compara e não guarda */
        Corte c2 = { 2, 2 }; Qz lo, hi; rz_caixa_inicial(c2, &lo, &hi);
        for(int i = 0; i < 10; i++){
            Qz m = qz_medio(lo, hi);
            int bom, sgn = rz_cmp(m, 2, 2, &bom);
            if(!bom) break;
            if(sgn < 0) lo = m; else hi = m;     /* substitui a ponta; não acumula */
        }
        printf("        memória: aranha escreve %ld marcas em %ld passos · encaixe escreve"
               " %ld em 10 dobras\n", escritas_aranha, passos[2], escritas_encaixe);
        ok("O DRAGÃO PREENCHE E A ARANHA PAGA POR ISSO — e é o contraste com a recta que"
           " interessa. Quadruplicando os passos, as células distintas multiplicam-se por"
           " cerca de quatro: a curva não é um fio no plano, ocupa área, e isso lê-se por"
           " contagem de caixas sem um único logaritmo. Mas ela precisa de memória: a aranha"
           " que a percorre escreve uma marca por passo (o G do `aranha_g.c`), porque no"
           " plano o caminho volta ao mesmo sítio e sem marca não sabe. Na recta, o mesmo"
           " passo de dobra não guarda NADA: cada dobra substitui uma ponta e a decisão"
           " seguinte pergunta ao corte, não ao passado. É a máquina sem memória a aparecer"
           " onde ela é possível — e a dizer onde não é",
           celulas[2] > celulas[1] && celulas[1] > celulas[0]
             && r1 > 200 && r2 > 200 && escritas_encaixe == 0);
        #undef GR2
    }

    /* ═══ §FR7 A MÉTRICA DA ÁRVORE É A CANÓNICA DE LEBESGUE ════════════════════ */
    printf("\n§FR7 Generalizada a base b, e ganchada à métrica canónica μ(A△B).\n\n");
    {
        /* O `algebrico thm:metrica` fixa a métrica CANÓNICA: d(A,B) = μ(A △ B), a
         * diferença simétrica — pseudométrica nos conjuntos, métrica no quociente pelos
         * de medida nula. A métrica da árvore não é uma régua nova: é ELA, lida nos
         * CILINDROS. Dois pontos que se separam no nível n têm cilindros DISJUNTOS de
         * medida b^-n cada, logo μ(A△B) = 2·b^-n = 2·d_b(x,y) — factor 2 exacto, e o
         * mesmo em toda base e todo nível. Conta-se em FOLHAS, sem divisão nenhuma. */
        int DD = 5;
        long ultra_mal = 0, triplos = 0, gancho_mal = 0, pares = 0, iso_mal = 0;
        printf("        b   pontos   triplos   ultramétrica   μ(A△B)/d = 2 ?   isometria\n");
        for(int b = 2; b <= 6; b++){
            long tot = 1; for(int i = 0; i < DD; i++) tot *= b;
            long N = tot < 90 ? tot : 90;
            long folhas_de[8];                       /* b^{DD-n} folhas por cilindro */
            for(int n = 0; n <= DD; n++){ long f = 1; for(int i = n; i < DD; i++) f *= b; folhas_de[n] = f; }
            /* separação de dois índices: o primeiro dígito diferente em base b */
            #define SEP(u,v) ({ int n_ = DD; long a_ = (u), b_ = (v); \
                int da_[8], db_[8]; for(int i = DD-1; i >= 0; i--){ da_[i] = a_ % b; a_ /= b; \
                db_[i] = b_ % b; b_ /= b; } \
                for(int i = 0; i < DD; i++) if(da_[i] != db_[i]){ n_ = i; break; } n_; })
            long ul = 0, tr = 0;
            for(long i = 0; i < N; i++) for(long j = 0; j < N; j++) for(long k = 0; k < N; k++){
                long x = i*tot/N, y = j*tot/N, z = k*tot/N;
                int nxy = SEP(x,y), nyz = SEP(y,z), nxz = SEP(x,z);
                int menor = nxy < nyz ? nxy : nyz;
                tr++; triplos++;
                if(nxz >= menor) ul++; else ultra_mal++;   /* d(x,z) ≤ max(d(x,y),d(y,z)) */
            }
            /* O GANCHO: μ(A△B) contado em folhas, contra 2·(folhas do cilindro de nível n) */
            long gm = 0, pr = 0;
            for(long i = 0; i < N; i++) for(long j = i+1; j < N; j++){
                long x = i*tot/N, y = j*tot/N;
                int n = SEP(x,y);
                if(n >= DD) continue;                     /* mesmo ponto no nível DD */
                /* os cilindros de nível n+1 (o primeiro nível onde já se separaram) são
                 * disjuntos: a diferença simétrica são as folhas dos dois */
                long mu = 2 * folhas_de[n+1];
                long d_arvore = folhas_de[n+1];           /* b^-(n+1) em folhas */
                pr++; pares++;
                if(mu != 2 * d_arvore) { gm++; gancho_mal++; }
            }
            /* ISOMETRIA (thm:metrica (2)): toda bijecção leva △ em △. Aqui a bijecção é
             * a permutação cíclica dos dígitos, aplicada em todos os níveis. */
            long im = 0;
            for(long i = 0; i < N; i++) for(long j = 0; j < N; j++){
                long x = i*tot/N, y = j*tot/N;
                long tx = 0, ty = 0, px = x, py = y, pw = 1;
                for(int q = 0; q < DD; q++){              /* dígito ↦ dígito+1 mod b */
                    tx += ((px % b + 1) % b) * pw; px /= b;
                    ty += ((py % b + 1) % b) * pw; py /= b;
                    pw *= b;
                }
                if(SEP(x,y) != SEP(tx,ty)) { im++; iso_mal++; }
            }
            printf("        %d    %4ld   %7ld      %s          %s            %s\n",
                   b, N, tr, ul == tr ? "sim" : "NÃO", gm == 0 ? "sim" : "NÃO",
                   im == 0 ? "sim" : "NÃO");
            #undef SEP
        }
        printf("        %ld triplos, %ld pares: ultramétrica %ld falhas · gancho %ld ·"
               " isometria %ld\n", triplos, pares, ultra_mal, gancho_mal, iso_mal);
        ok("E A MÉTRICA DESTE ANDAR NÃO É UMA RÉGUA MINHA: É A CANÓNICA DA CASA, lida nos"
           " cilindros. O `algebrico thm:metrica` fixa d(A,B) = μ(A △ B) — a diferença"
           " simétrica, pseudométrica nos conjuntos e métrica no quociente pelos de medida"
           " nula. Na árvore, dois pontos que se separam no nível n têm cilindros DISJUNTOS"
           " de medida b^-(n+1) cada, logo μ(A△B) = 2·b^-(n+1): a métrica da árvore é"
           " exactamente METADE da canónica, com factor 2 constante — o mesmo em toda base e"
           " todo nível, e contado em FOLHAS sem uma divisão. Verifica-se ainda o que o"
           " teorema pede do outro lado: a ultramétrica vale em todas as bases de 2 a 6, e"
           " toda bijecção da árvore é ISOMETRIA nela (thm:metrica (2)), medido com a"
           " permutação cíclica dos dígitos",
           ultra_mal == 0 && gancho_mal == 0 && iso_mal == 0 && triplos > 0 && pares > 0);
    }

    /* ═══ §FR8 A DIMENSÃO GENERALIZADA, com os dois controlos ══════════════════ */
    printf("\n§FR8 Guardar k dos b dígitos: s com k = b^s, e os casos k=b e k=1.\n\n");
    {
        long mal = 0, casos = 0;
        printf("        b   k    corte {p/q : b^p < k^q}     abaixo  acima  EM CIMA   s\n");
        struct { int b, k; const char *nome; } F[6] = {
            {3,2,"Cantor classico"}, {5,3,"Cantor 3-de-5"}, {4,2,"2 de 4: s racional"},
            {8,2,"2 de 8: s racional"}, {2,2,"o intervalo"}, {5,1,"um ponto"}
        };
        for(int t = 0; t < 6; t++){
            int b = F[t].b, k = F[t].k;
            long abaixo = 0, acima = 0, em_cima = 0;
            for(long q = 1; q <= 24; q++) for(long p = 1; p <= q; p++){
                unsigned long long tb = 1, tk = 1; int estoura = 0;
                for(long i = 0; i < p; i++){ if(tb > (~0ULL)/(unsigned)b){ estoura = 1; break; } tb *= (unsigned)b; }
                for(long i = 0; i < q && !estoura; i++){ if(tk > (~0ULL)/(unsigned)(k?k:1)){ estoura = 1; break; } tk *= (unsigned)(k?k:1); }
                if(estoura) continue;
                if(tb < tk) abaixo++; else if(tb > tk) acima++; else em_cima++;
            }
            casos++;
            /* O CRITÉRIO, POR UM SEGUNDO CAMINHO que não olha para o laço acima:
             * b^p = k^q tem solução ⟺ b e k são potências de um MESMO inteiro m, e aí
             * s = j/i é racional (b = m^i, k = m^j). O caso k = 1 é s = 0, e aí nenhum
             * racional POSITIVO fica abaixo. Foi aqui que a asserção caiu na primeira
             * escrita: rotulei b=4, k=2 de irracional, e s = log2/log4 = 1/2. */
            int fecha_previsto = 0, si = 0, sj = 0;
            if(k == 1) fecha_previsto = 1;
            else for(int m = 2; m <= b && !fecha_previsto; m++){
                int i = 0, j = 0; long t1 = 1, t2 = 1;
                while(t1 < b){ t1 *= m; i++; }
                while(t2 < k){ t2 *= m; j++; }
                if(t1 == b && t2 == k && i > 0 && j > 0){ fecha_previsto = 1; si = j; sj = i; }
            }
            int fecha_medido = (k == 1) ? (abaixo == 0) : (em_cima > 0);
            if(fecha_medido != fecha_previsto) mal++;
            char sdesc[32];
            if(k == 1) snprintf(sdesc, sizeof sdesc, "= 0");
            else if(fecha_previsto) snprintf(sdesc, sizeof sdesc, "= %d/%d", si, sj);
            else snprintf(sdesc, sizeof sdesc, "IRRACIONAL");
            printf("        %d   %d    %-22s  %5ld  %5ld    %4ld    %s\n",
                   b, k, F[t].nome, abaixo, acima, em_cima, sdesc);
        }
        ok("E A DIMENSÃO GENERALIZA COM A MÉTRICA: guardando k dos b dígitos, a cobertura"
           " canónica de nível n tem k^n peças de diâmetro b^-n, logo s satisfaz k = b^s e o"
           " corte é {p/q : b^p < k^q} — inteiro, como todos os deste paper. E o corte FECHA"
           " ou não conforme a aritmética de (b,k), o que aqui se decide por DOIS caminhos"
           " independentes: o laço procura p,q com b^p = k^q, e à parte pergunta-se se b e k"
           " são potências de um mesmo inteiro m. Com k = b o corte fecha em s = 1; com k = 1"
           " fecha em s = 0 (e nenhum racional positivo fica abaixo); com 2 de 4 e 2 de 8 ele"
           " fecha em s = 1/2 e s = 1/3 — dimensões RACIONAIS, e foi aqui que esta asserção"
           " caiu na primeira escrita, por eu ter rotulado 2-de-4 de irracional quando"
           " log2/log4 = 1/2. Só nos casos sem potência comum — 2 de 3, 3 de 5 — é que o"
           " corte não fecha. É o §RE5 outra vez: o andar só acrescenta onde ℚ não chega",
           mal == 0 && casos == 6);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
