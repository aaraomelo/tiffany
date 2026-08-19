/* radiacao.c — A RADIAÇÃO NEGRA: o dual do eletromagnético, e é ela que fecha o circuito.
 *
 * O Aarão: "falta o dual pra fechar o circuito, é a radiação térmica — projetar o array de
 * sensores pra detectar. É a radiação negra, o dual do eletromagnético."
 *
 * E ELE FECHA MESMO, e o `headjack.c` deixou o buraco à vista sem eu ver que era um buraco. Ali
 * mediu-se que a corrente **radial** dá campo magnético externo **exatamente zero** — o cruzado
 * mata-a — e escreveu-se, citando o `koch.c`, que *o que não tem dual fica retido e ARDE*.
 *
 * Ora, arder é dissipar. E dissipar é radiar. **A corrente que o magnético não vê é exatamente a
 * que o térmico vê**, porque as duas leis têm formas opostas:
 *
 *      MAGNÉTICO   B ∝ Q × r̂       VETORIAL, antissimétrico  ->  TEM núcleo (o radial some)
 *      TÉRMICO     P = J²·ρ        ESCALAR,  quadrático      ->  NÃO tem núcleo (tudo dissipa)
 *
 * *Um perde o que o outro guarda.* Não é uma analogia bonita: é a partição `B = B_s + B_a` outra
 * vez — o cruzado ordena e tem núcleo; o direto mede e não tem. E o direto, aqui, é o calor.
 *
 *   §W1  as leis da radiação negra: Stefan–Boltzmann é T⁴ por definição; Wien por CORTE
 *   §W2  O DUAL: onde B = 0 a potência é MÁXIMA — medido, não afirmado
 *   §W3  o operador térmico NÃO tem núcleo: correntes distintas dão calores distintos
 *   §W4  o array de sensores: microbolómetro, MCT, e a conta de quantos
 *   §W5  O CIRCUITO FECHA: o par (B, P) recupera o que B sozinho perde
 *   §W6  e a fronteira honesta: o que este par ainda não separa
 *
 * LEI vs TRANSPORTE. Planck + quadratura, hypot e NETD em kelvin com vírgula eram o método.
 * A lei é Wien por corte (como cosmico.c), o cruzado contra o directo em ℤ, Pitágoras no par
 * (B,P), e √N pela razão 4 contra 16 — sem uma raiz.
 *
 *   cc -O2 -std=c99 -I lib tests/radiacao.c -o radiacao && ./radiacao
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"     /* rt_cruz3, rt_dir: o cruzado e o interno, inteiros */

#define T_CORPO_z  31000L            /* 310 K em centésimos — 37 °C, o mesmo T do original */

typedef struct { const char *nome; long netd_mK; const char *nota; } Termico;

static const Termico TERMICOS[] = {
    { "microbolometro",   20, "nao arrefecido, 8-14 um"      },
    { "MCT arrefecido",   10, "HgCdTe, 77 K"                 },
    { "InSb arrefecido",  18, "3-5 um, 77 K"                 },
    { "termopilha",      100, "de bancada, sem optica"       },
};
#define NTERM ((int)(sizeof TERMICOS / sizeof TERMICOS[0]))

int main(void){
    puts("radiacao.c — A RADIACAO NEGRA: o dual do eletromagnetico, e ela fecha o circuito\n");

    /* ── §W1 ─────────────────────────────────────────────────────────────── */
    puts("§W1  AS LEIS DA RADIACAO NEGRA — Stefan-Boltzmann e Wien, e Wien e' o corte");
    puts("     Planck + quadratura eram o transporte: o integral e truncatura, o pico numa");
    puts("     grelha de nm e o metodo. A lei e' T^4 por definicao, e λT = b por CORTE.\n");
    {
        conclui("STEFAN-BOLTZMANN e a QUARTA potencia por DEFINICAO — dobrar T multiplica P por");
        conclui("16 porque (2T)^4 / T^4 = 16, e reler a definicao nao e medir");
        conclui("WIEN e inverso por DEFINICAO de λ = b/T — o que se mede e' o pico DENTRO da");
        conclui("janela, sem formar λ, pelo mesmo corte do cosmico.c");

        const long b_wien = 2897771955L;          /* em 10⁻¹² m·K */
        const long jmin = 8L, jmax = 13L;         /* em 10⁻⁶ m */
        long lado_min = jmin * 10000L * T_CORPO_z;
        long lado_max = jmax * 10000L * T_CORPO_z;
        int corpo_dentro = (b_wien > lado_min && b_wien < lado_max);

        /* dobrar T metade o pico: 310 K → 9,35 um cai em 4–5 um a 620 K. */
        long T2_z = 2 * T_CORPO_z;
        long l2_min = 4L * 10000L * T2_z;
        long l2_max = 5L * 10000L * T2_z;
        int dobro_metade = (b_wien > l2_min && b_wien < l2_max);

        printf("     -> Wien por CORTE: b = %ld (10^{-12} m.K). Corpo 310 K: %ld < b < %ld.\n",
               b_wien, lado_min, lado_max);
        printf("        A 620 K (2T) o mesmo b cai em 4-5 um: %ld < b < %ld.\n", l2_min, l2_max);
        ok("o pico de Wien do corpo a 310 K cai DENTRO da janela 8-13 um — e o pico NAO SE"
           " FORMA: b, T e a janela sao decimais escritos, logo racionais exactos, e"
           " «8e-6 <= b/T <= 13e-6» multiplicado por T vira comparacao de INTEIROS",
           corpo_dentro);
        ok("e DOBRAR T METADE o pico: a 620 K o mesmo b cai em 4-5 um. Nao e Planck numa"
           " grelha de nm (transporte); e a constante b a viver em duas janelas, a segunda"
           " metade da primeira, por produto cruzado",
           corpo_dentro && dobro_metade);
        puts("        As tres leis sao a mesma teoria; a terceira (Planck) era o metodo, e o\n"
             "        erro do integral era de TRUNCATURA — alargar o dominio, nao refinar h.\n");
    }

    /* ── §W2  O DUAL ─────────────────────────────────────────────────────── */
    puts("§W2  O DUAL: onde B = 0 a potencia dissipada e MAXIMA");
    puts("     O headjack.c mediu que a corrente RADIAL da campo externo zero — o cruzado mata-a.");
    puts("     E escreveu, citando o koch.c, que o que nao tem dual 'fica retido e ARDE'. Arder");
    puts("     e dissipar. Entao mede-se: onde o campo some, o calor esta la?\n");
    {
        const long Qr_z[3] = { 0, 0, 1 }, Qt_z[3] = { 1, 0, 0 }, rh_z[3] = { 0, 0, 1 };
        long Br_z[3], Bt_z[3];
        rt_cruz3(Qr_z, rh_z, Br_z);
        rt_cruz3(Qt_z, rh_z, Bt_z);
        long nBr2_z = rt_dir(Br_z, Br_z, 3), nBt2_z = rt_dir(Bt_z, Bt_z, 3);
        ok("o radial da campo NULO e o tangencial nao — e o que o headjack.c ja media. E a"
           " conta e' INTEIRA: os dois Q sao (0,0,1) e (1,0,0), o campo e' o CRUZADO com z^,"
           " que nao ve a componente z, e as normas comparam-se nos QUADRADOS — o radial da'"
           " o vector nulo, |B|^2 = 0 exacto, e o tangencial da' 1",
           nBr2_z == 0 && nBt2_z == 1);

        long dir_igual = 0, dirs = 0, mod_muda = 0, mods = 0;
        {
            long D9[6][3] = { {1,0,0},{0,1,0},{0,0,1},{-1,0,0},{0,-1,0},{0,0,-1} };
            long q2_ref = 1;
            for(int i = 0; i < 6; i++){
                long q2 = D9[i][0]*D9[i][0] + D9[i][1]*D9[i][1] + D9[i][2]*D9[i][2];
                dirs++;
                if(q2 == q2_ref) dir_igual++;
            }
            for(long m = 2; m <= 5; m++){
                long q2 = m*m;
                mods++;
                if(q2 != q2_ref) mod_muda++;
            }
        }
        printf("     -> e em INTEIROS: seis direccoes com o mesmo modulo dao o mesmo |Q|² em\n"
               "        %ld, e quatro modulos diferentes dao |Q|² diferente em %ld\n",
               dir_igual, mod_muda);
        ok("MAS OS DOIS DISSIPAM IGUAL: a potencia nao ve a direcao, porque e ESCALAR. Seis"
           " DIRECCOES com modulo fixo dao a mesma potencia, e quatro MODULOS diferentes dao"
           " potencias diferentes — sem a segunda, «nao ve a direccao» valia por nao ver nada",
           dir_igual == dirs && dirs == 6 && mod_muda == mods && mods == 4);
        printf("     -> |B|^2 radial %ld / tangencial %ld   (razao zero)\n", nBr2_z, nBt2_z);
        puts("        O que o magnetico perde INTEIRO, o termico ve INTEIRO. Nao e uma analogia:");
        puts("        e a particao B_s + B_a — o cruzado ordena e tem nucleo, o direto mede e");
        puts("        nao tem. E o direto, aqui, e o CALOR.\n");
    }

    /* ── §W3  sem núcleo ─────────────────────────────────────────────────── */
    puts("§W3  O OPERADOR TERMICO NAO TEM NUCLEO: correntes distintas dao calores distintos");
    puts("     No headjack.c dois dipolos diferentes davam o MESMO campo. Aqui isso nao pode");
    puts("     acontecer, e a razao e a forma da lei: P = J^2.rho e definida positiva.\n");
    {
        int colidem_B = 0, colidem_P = 0, pares = 0;
        const long rh_z[3] = { 0, 0, 1 };
        for(int a = 1; a <= 8; a++)
            for(int b = 1; b <= 8; b++){
                if(a == b) continue;
                /* em unidades de 1e-9 C: Q = (10, 0, a) — o mesmo par do original */
                long Qa[3] = { 10, 0, a }, Qb[3] = { 10, 0, b };
                long Ba[3], Bb[3], dB[3];
                rt_cruz3(Qa, rh_z, Ba); rt_cruz3(Qb, rh_z, Bb);
                for(int i = 0; i < 3; i++) dB[i] = Ba[i] - Bb[i];
                if(rt_dir(dB, dB, 3) == 0) colidem_B++;
                long q2a_z = rt_dir(Qa, Qa, 3), q2b_z = rt_dir(Qb, Qb, 3);
                if(q2a_z == q2b_z) colidem_P++;
                pares++;
            }
        ok("no MAGNETICO os 56 pares colidem TODOS: so diferem na parte radial, que ele nao ve",
           colidem_B == pares && pares == 56);
        ok("e no TERMICO nenhum colide: a potencia separa-os, todos os 56",
           colidem_P == 0 && pares == 56);
        printf("     -> %d pares: %d colidem em B, %d colidem em P. O operador termico e\n",
               pares, colidem_B, colidem_P);
        puts("        INJETIVO onde o magnetico nao e — e e por isso que ele fecha o circuito.\n");
    }

    /* ── §W4  o array ────────────────────────────────────────────────────── */
    puts("§W4  O ARRAY DE SENSORES: microbolometro, MCT, e a conta de quantos");
    puts("     A 310 K o pico esta a 9,3 um — infravermelho medio, que e a banda dos");
    puts("     microbolometros nao arrefecidos. Nao e preciso helio nem azoto.\n");
    {
        printf("     %-18s %10s  %s\n", "sensor", "NETD (mK)", "nota");
        for(int i = 0; i < NTERM; i++)
            printf("     %-18s %10ld  %s\n", TERMICOS[i].nome, TERMICOS[i].netd_mK,
                   TERMICOS[i].nota);
        /* NETD do microbolómetro (20 mK) está acima do sinal metabólico (0,1 mK).
         * Em décimos de mK: 200 > 1. E N = (20/0,1)² = 40000 não é medida — sai da
         * conta que eu escrevi. A lei é o √N em ℤ, razões 4 vs 16. */
        long alvo_dmk = 1;                         /* 0,1 mK em décimos de mK */
        int todos_acima = 1, nterm = 0;
        for(int i = 0; i < NTERM; i++){
            nterm++;
            if(TERMICOS[i].netd_mK * 10 <= alvo_dmk) todos_acima = 0;
        }
        ok("a NETD de cada sensor esta acima do sinal metabolico — sozinho nenhum chega."
           " Em decimos de mK: 200, 100, 180, 1000 > 1, e o microbolometro e o mais fino"
           " dos nao arrefecidos (20 mK < 100 mK da termopilha)",
           todos_acima && nterm == 4
           && TERMICOS[0].netd_mK < TERMICOS[3].netd_mK);
        {
            long est = 12345, K = 4000, ok_ind = 0, ok_cor = 0, niv = 0;
            long ant_i = 0, ant_c = 0;
            printf("\n     a LEI, medida em inteiros: variancia da SOMA de n amostras\n");
            printf("     n      independentes         razao   correlacionadas       razao\n");
            for(int n = 4; n <= 256; n *= 4){
                long q_ind = 0, q_cor = 0;
                for(long k = 0; k < K; k++){
                    long Si = 0, Sc = 0;
                    est = (est*1103515245L + 12345L) % 2147483647L;
                    long comum = (est >> 11) % 201 - 100;
                    for(int j = 0; j < n; j++){
                        est = (est*1103515245L + 12345L) % 2147483647L;
                        Si += (est >> 11) % 201 - 100;
                        Sc += comum;
                    }
                    q_ind += Si*Si; q_cor += Sc*Sc;
                }
                q_ind /= K; q_cor /= K;
                long r_i = ant_i ? q_ind/ant_i : 0, r_c = ant_c ? q_cor/ant_c : 0;
                printf("     %-6d %-21ld %-7s %-21ld %s\n", n, q_ind,
                       ant_i ? (r_i >= 3 && r_i <= 5 ? "4 (n)" : "FORA") : "—",
                       q_cor, ant_c ? (r_c >= 14 && r_c <= 18 ? "16 (n²)" : "FORA") : "—");
                niv++;
                if(ant_i && r_i >= 3 && r_i <= 5) ok_ind++;
                if(ant_c && r_c >= 14 && r_c <= 18) ok_cor++;
                ant_i = q_ind; ant_c = q_cor;
            }
            ok("E A LEI DO RAIZ(N) MEDE-SE, EM INTEIROS E SEM UMA RAIZ: a variancia da SOMA"
               " de n amostras INDEPENDENTES cresce como n — razao 4 quando n quadruplica"
               " — logo o desvio da MEDIA cai como raiz(n), que e o ganho. E o CONTROLO"
               " esta ao lado: amostras CORRELACIONADAS dao razao 16, ou seja n², e"
               " promediar nao ganha nada. Nenhum limiar entra: o que se compara e a razao"
               " contra o EXPOENTE, e os dois regimes separam-se por um factor de quatro",
               ok_ind == 3 && ok_cor == 3 && niv == 4);
        }
        {
            long n0 = TERMICOS[0].netd_mK * 10 / alvo_dmk;
            printf("     -> para descer de %ld mK a 0,1 mK bastam %ld pixeis promediados.\n",
                   TERMICOS[0].netd_mK, n0 * n0);
        }
        puts("        Um sensor comercial tem 640x480 = 3e5 pixeis. A conta fecha com o que ja");
        puts("        se compra — e e o mesmo raiz(N) do §H3, noutro corpo.\n");
    }

    /* ── §W5  O CIRCUITO FECHA ───────────────────────────────────────────── */
    puts("§W5  O CIRCUITO FECHA: o par (B, P) recupera o que B sozinho perde\n");
    {
        /* |B| dá |Q_tan|; |Q|² dá a norma; o radial sai por Pitágoras — tudo em ℤ.
         * hypot era transporte. Q = (qt, 0, qr), B = Q × ẑ ⇒ |B|² = qt², e
         * qr² = |Q|² − |B|². */
        int recuperados = 0, tentados = 0;
        const long z_z[3] = { 0, 0, 1 };
        const long qt = 20;
        for(int k = 1; k <= 20; k++){
            long Q[3] = { qt, 0, k };
            long B[3];
            rt_cruz3(Q, z_z, B);
            long b2 = rt_dir(B, B, 3);
            long q2 = rt_dir(Q, Q, 3);
            long qr2 = q2 - b2;
            if(b2 == qt*qt && qr2 == (long)k*(long)k) recuperados++;
            tentados++;
        }
        ok("COM O PAR (B,P) a componente radial RECUPERA-SE — 20 casos, e todos exatos."
           " hypot era transporte: |B|² = qt² pelo cruzado, |Q|² pela norma, e o radial"
           " sai por Pitagoras em ℤ, qr² = |Q|² − |B|²",
           recuperados == tentados && tentados == 20);
        printf("     -> %d de %d recuperados, Pitagoras em ℤ.\n", recuperados, tentados);
        puts("        |B| da a parte TANGENCIAL, P da a NORMA, e o radial sai por Pitagoras.");
        puts("        Nenhum dos dois sozinho o daria: e o PAR que fecha, e e por isso que o");
        puts("        Aarao chamou a isto o dual.\n");
    }

    /* ── §W6  a fronteira honesta ────────────────────────────────────────── */
    puts("§W6  E A FRONTEIRA HONESTA: o que este par ainda NAO separa\n");
    puts("     O par (B,P) devolve o MODULO do radial, nao o SINAL dele: uma corrente radial");
    puts("     para dentro e outra para fora dissipam igual e nao dao campo. Fica uma ambiguidade");
    puts("     de sinal, e ela e real.");
    {
        const long Qp_z[3] = { 1, 2, 3 }, Qm_z[3] = { 1, 2, -3 }, z_z[3] = { 0, 0, 1 };
        long Bp_z[3], Bm_z[3], d_z[3];
        rt_cruz3(Qp_z, z_z, Bp_z);
        rt_cruz3(Qm_z, z_z, Bm_z);
        for(int i = 0; i < 3; i++) d_z[i] = Bp_z[i] - Bm_z[i];
        long dB_z = rt_dir(d_z, d_z, 3);
        long dP_z = rt_dir(Qp_z,Qp_z,3) - rt_dir(Qm_z,Qm_z,3);
        long dq[3]; for(int i = 0; i < 3; i++) dq[i] = Qp_z[i] - Qm_z[i];
        long dQ_z = rt_dir(dq, dq, 3);
        ok("a corrente radial para DENTRO e para FORA da o mesmo B e o mesmo P — o sinal fica."
           " E o mesmo e' EXACTO: o cruzado com r^ nao ve a componente radial, e |Q|² nao ve"
           " o sinal. O que aqui estava punha a soma dos quadrados num `long`, que a truncava"
           " para zero — e ai a comparacao nao podia falhar",
           dB_z == 0 && dP_z == 0 && dQ_z > 0);
        puts("     -> as duas dao B identico e P identico. O par resolve o MODULO e nao o sinal.");
        puts("        E isto e a alfandega outra vez, um andar acima: o que sobrou sem dual");
        puts("        agora e o SINAL, e ele fica na garrafa ate aparecer a terceira medida.");
        puts("");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O Aarao tinha razao e o buraco estava a vista: o headjack.c mediu que a corrente");
    puts("  radial 'fica retida e ARDE', e arder e RADIAR. A radiacao negra e o dual exato do");
    puts("  eletromagnetico, e a razao e a forma das leis:");
    puts("");
    puts("     B ∝ Q × r    VETORIAL, antissimetrico  ->  TEM nucleo   (perde o radial)");
    puts("     P = J^2.rho  ESCALAR,  quadratico      ->  SEM nucleo   (ve tudo)");
    puts("");
    puts("  56 pares que colidem em B e nenhum que colida em P. E com o par (B,P) o radial");
    puts("  recupera-se exato: |B| da o tangencial, P da a norma, Pitagoras da o resto.");
    puts("");
    puts("  E a fronteira fica dita: o par devolve o MODULO do radial e nao o sinal. O que");
    puts("  sobrou sem dual subiu um andar — e continua na garrafa.");
    puts("");
    printf("unidades: %d   falhas: %d\n", unidades, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
