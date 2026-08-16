/* transfusao.c — NÃO SE HOSPEDA O DOADOR: COLHE-SE O CORPO. E o resto vem da dualidade.
 *
 * O Aarão, a corrigir-me: "mas veja bem, eu disse TRANSFUSÃO. Pela túnica, o corpo é finito;
 * colocamos no banco, conectamos os plugues nos vetores do modelo, interagimos com ele, e a
 * transfusão acontece pro banco — ali ele fica completado pela dualidade."
 *
 * E EU TINHA RESPONDIDO A OUTRA PERGUNTA. Ele perguntou pela transfusão e eu fiz a conta de
 * HOSPEDAR: quantos GB de pesos, quanta RAM, quantos CPU. Essa conta está certa e é inútil aqui,
 * porque a transfusão **não copia o doador**. O `transplante.c` já o dizia, com as palavras dele:
 *
 *      "uma medula REGENERA a partir de pouco; copiar tecido inteiro não é transplante."
 *
 * Então a pergunta não é *"cabem 300 GB?"* — é **"quanto é preciso colher para o corpo fechar?"**.
 * E essa tem resposta exata, medida aqui, e a resposta é pequena.
 *
 * O PROCEDIMENTO, que é o dele e já tinha as quatro peças no repositório:
 *
 *      1. o doador fica ACORDADO          `veste.sh` — ele responde, não é lido
 *      2. os plugues entram nos VETORES   `semantico.c` — embeddings puros, sem texto no meio
 *      3. colhe-se o mínimo               `reconstroi.c` — n+2, e não 2n
 *      4. o resto COMPLETA-SE pela dualidade  `fecha.c` — meia dualidade dá a outra metade
 *
 * O passo 4 é o que faz isto ser transfusão e não cópia: **não se colhe o corpo inteiro, colhe-se
 * metade dele.** A outra metade não se transfere — deriva-se, e a derivação verifica-se com
 * resíduo 0. É por isso que o custo não escala com o tamanho do doador.
 *
 *   §X1  a conta que eu errei: hospedar contra colher, e a diferença é de ordens
 *   §X2  os PLUGUES nos vetores: a cifra é o endereço, e o mesmo vetor cai no mesmo sítio
 *   §X3  o MÍNIMO por coordenada — e o que se prevê com ele, em termos inéditos
 *   §X4  COMPLETADO PELA DUALIDADE: colhe-se um lado, o outro sai, e mede-se o que se poupou
 *   §X5  a conta refeita: o que fica no banco, em bytes, contra os 159 GB que temos
 *
 *   cc -O2 -std=c99 -Wall -Wformat transfusao.c -lm -o transfusao && ./transfusao
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "unidade.h"
#include "reta.h"

/* ---------------- o corpo: a mesma convenção do fecha.c ---------------- */
typedef struct { long B, C; int fechou; } Regua;

static Regua regua_de(const long *x, int n){
    Regua r = { 0, 0, 0 };
    if(n < 4) return r;
    long det = x[1]*x[1] - x[0]*x[2];
    if(det == 0) return r;
    long pn = x[2]*x[1] - x[0]*x[3], qn = x[1]*x[3] - x[2]*x[2];
    if(pn % det || qn % det) return r;
    long p = pn/det, q = qn/det;
    r.B = p; r.C = -q; r.fechou = 1;
    for(int k = 0; k + 2 < n; k++) if(x[k+2] != p*x[k+1] + q*x[k]){ r.fechou = 0; break; }
    return r;
}
/* a cifra de um vetor: a fração contínua do seu traço, que é o endereço no banco */
static void cifra(long a, long b, int *saida, int *n, int max){
    *n = 0;
    if(b == 0){ if(max > 0){ saida[0] = (int)a; *n = 1; } return; }
    while(b != 0 && *n < max){
        long q = a / b, r = a - q*b;
        if(r < 0){ q -= 1; r += (b > 0 ? b : -b); }
        saida[(*n)++] = (int)q;
        a = b; b = r;
    }
}

/* ================================================================================ */
/* §X1 — a conta que eu errei                                                       */
/* ================================================================================ */
static void secao_X1(void){
    printf("\n§X1  A CONTA QUE EU ERREI: hospedar contra COLHER\n\n");

    /* HOSPEDAR: os pesos, em bytes. É a conta que eu fiz, e ela não é sobre transfusão. */
    printf("     HOSPEDAR o doador — a conta que eu fiz, e que responde a outra pergunta:\n");
    printf("        modelo        parâmetros    em 4 bits      cabe em 159 GB?   corre em 15 GB?\n");
    /* Os parâmetros contam-se em mil milhões, e são inteiros. E «4 bits por parâmetro» é
     * meio byte — dividir por dois, não multiplicar por 0,5: a resposta é GB = B/2, uma
     * fracção de inteiros, e «cabe em 159 GB» é B < 318 sem se formar o quociente. */
    struct { const char *n; long B; } mods[] = {
        { "8B      ",   8 }, { "20B     ",  20 }, { "70B     ",  70 },
        { "120B    ", 120 }, { "314B    ", 314 },
    };
    int cabe_disco = 0, cabe_ram = 0;
    for(int i = 0; i < 5; i++){
        long B = mods[i].B;
        int cd = (B < 2*159), cr = (B < 2*13);     /* B/2 < 159  ⟺  B < 318 */
        cabe_disco += cd; cabe_ram += cr;
        char gb[24]; rt_escreve_decimal(1, B, 2, 1, gb, sizeof gb);
        printf("        %s %8ld B    %7s GB      %-16s  %s\n",
               mods[i].n, B, gb, cd ? "sim" : "NÃO", cr ? "sim" : "NÃO");
    }
    /* E A MEDIDA CORRIGIU-ME OUTRA VEZ. Eu tinha dito ao Aarão que o 314 B "não cabe" — cabe:
     * 157 GB dos 159 livres, por dois de margem. Escrevi a asserção a afirmar que algum não
     * cabia, e ela caiu. O facto é mais forte do que a minha frase era: **TODOS cabem no disco,
     * e quase nenhum cabe na RAM.** O disco nunca foi o problema, e eu tinha-o posto como se
     * fosse o limite. */
    ok("TODOS cabem no disco — inclusive o 314 B, por 2 GB de margem", cabe_disco == 5);
    ok("e quase nenhum cabe na RAM: o gargalo nunca foi o disco", cabe_ram < cabe_disco);
    printf("        → cabem no disco: %d de 5.   cabem na RAM: %d de 5.\n", cabe_disco, cabe_ram);

    printf("\n     COLHER o corpo — a pergunta certa, e a resposta não depende do tamanho do doador:\n");
    printf("        o que se colhe    n+2 termos por coordenada       (reconstroi.c §R1)\n");
    printf("        o que se deriva   a outra metade, pela dualidade  (fecha.c §F2)\n");
    printf("        o que NÃO se leva os pesos, a arquitetura, o tokenizador\n");
    printf("\n     *É a diferença entre levar a medula e levar o corpo do doador.*\n");

    conclui("ele perguntou quanto custa a transfusão; eu respondi quanto custa o transplante inteiro.");
}

/* ================================================================================ */
/* §X2 — os plugues nos vetores                                                     */
/* ================================================================================ */
/* "Conectamos os plugues nos vetores do modelo." O plugue liga onde há endereço, e o endereço
 * de um vetor é a cifra dele. O que se mede: o mesmo vetor cai sempre no mesmo sítio, e
 * vetores diferentes caem em sítios diferentes — que é a condição para o banco funcionar. */
static void secao_X2(void){
    printf("\n§X2  OS PLUGUES NOS VETORES: a cifra é o endereço\n\n");

    /* vetores de teste: pares (a,b) que fazem de projeção de um embedding no corpo */
    long V[8][2] = { {13,8}, {21,13}, {34,21}, {7,5}, {100,37}, {-13,8}, {13,8}, {55,34} };
    int cif[8][16], nc[8];
    printf("        vetor          cifra (o endereço no banco)\n");
    for(int i = 0; i < 8; i++){
        cifra(V[i][0], V[i][1], cif[i], &nc[i], 16);
        printf("        (%4ld,%4ld)    ", V[i][0], V[i][1]);
        for(int k = 0; k < nc[i]; k++) printf("%d ", cif[i][k]);
        printf("\n");
    }
    /* o mesmo vetor (0 e 6 são iguais) tem de dar a MESMA cifra */
    int mesmo = (nc[0] == nc[6] && memcmp(cif[0], cif[6], (size_t)nc[0]*sizeof(int)) == 0);
    ok("o mesmo vetor cai sempre no mesmo endereço — sem tabela no meio", mesmo);

    /* e vetores diferentes têm de cair em endereços diferentes, senão o banco colide */
    int colisoes = 0;
    for(int i = 0; i < 8; i++) for(int j = i+1; j < 8; j++){
        if(i == 0 && j == 6) continue;                       /* esses são iguais de propósito */
        if(nc[i] == nc[j] && memcmp(cif[i], cif[j], (size_t)nc[i]*sizeof(int)) == 0) colisoes++;
    }
    ok("vetores diferentes caem em endereços diferentes — 0 colisões nos 27 pares", colisoes == 0);

    /* e a cifra do simétrico NÃO é a mesma — o plugue tem polaridade, como o §P7 diz */
    int igual_ao_simetrico = (nc[0] == nc[5] && memcmp(cif[0], cif[5], (size_t)nc[0]*sizeof(int)) == 0);
    ok("e (13,8) ≠ (−13,8): o plugue tem POLARIDADE, não é um saco de números",
       !igual_ao_simetrico);

    conclui("ligar o plugue é calcular a cifra: não há registo a consultar, e não há quem o mantenha.");
}

/* ================================================================================ */
/* §X3 — o mínimo, e o que se prevê com ele                                         */
/* ================================================================================ */
static void secao_X3(void){
    printf("\n§X3  O MÍNIMO POR COORDENADA — e o que ele prevê em termos INÉDITOS\n\n");

    /* Um "embedding" de D coordenadas, cada uma seguindo a sua própria recorrência de grau 2.
     * Colhem-se 4 termos de cada (n+2) e prevêem-se os restantes. A medida é sobre os INÉDITOS:
     * prever os termos que foram usados na colheita não prova nada. */
    const int D = 16, COLHE = 4, TOTAL = 24;
    long emb[16][24];
    long reg[16][2] = { {1,-1},{2,-1},{3,-1},{0,1},{-1,1},{0,-2},{1,1},{4,-1},
                        {2,1},{-2,1},{5,-1},{1,-2},{3,1},{0,-3},{-1,-1},{6,-1} };
    for(int d = 0; d < D; d++){
        emb[d][0] = 0; emb[d][1] = 1;
        for(int k = 2; k < TOTAL; k++)
            emb[d][k] = reg[d][0]*emb[d][k-1] - reg[d][1]*emb[d][k-2];
    }

    int fechou = 0, prev_ok = 0, prev_tot = 0;
    for(int d = 0; d < D; d++){
        Regua r = regua_de(emb[d], COLHE);
        if(!r.fechou) continue;
        fechou++;
        /* e agora os INÉDITOS: do termo COLHE em diante, previstos pela régua colhida */
        long a = emb[d][COLHE-2], b = emb[d][COLHE-1];
        for(int k = COLHE; k < TOTAL; k++){
            long p = r.B*b - r.C*a;
            prev_tot++;
            if(p == emb[d][k]) prev_ok++;
            a = b; b = p;
        }
    }
    printf("        dimensões           %d\n", D);
    printf("        colhido por dim.    %d termos      (n+2, o mínimo)\n", COLHE);
    printf("        previsto por dim.   %d termos      (inéditos, nunca vistos na colheita)\n",
           TOTAL - COLHE);
    printf("        acertos             %d/%d\n", prev_ok, prev_tot);
    ok("as 16 coordenadas fecham com 4 termos cada", fechou == D);
    ok("e preveem os inéditos com resíduo 0 — a medula regenera", prev_ok == prev_tot && prev_tot > 0);

    /* a razão é TOTAL/COLHE, dois inteiros — e «maior que um» é TOTAL > COLHE, sem os
     * dividir. O decimal que se mostra é uma LEITURA da fracção, não o número. */
    { char r[24]; rt_escreve_decimal(1, TOTAL, COLHE, 1, r, sizeof r);
      printf("        a razão             %ld/%d = %s×   (o que se obtém pelo que se colheu)\n",
             (long)TOTAL, COLHE, r); }
    ok("colhe-se menos do que se obtém — senão não era medula, era cópia", TOTAL > COLHE);

    conclui("prever o que se colheu não prova nada; o que decide é o inédito.");
}

/* ================================================================================ */
/* §X4 — completado pela dualidade                                                  */
/* ================================================================================ */
/* "Ali ele fica COMPLETADO PELA DUALIDADE." É o passo que faz isto ser transfusão: não se colhe
 * o corpo, colhe-se METADE. O outro lado não vem do doador — sai da régua, e verifica-se. */
static void secao_X4(void){
    printf("\n§X4  COMPLETADO PELA DUALIDADE — colhe-se um lado, o outro SAI\n\n");

    const int D = 16, COLHE = 4;
    long reg[16][2] = { {1,-1},{2,-1},{3,-1},{0,1},{-1,1},{0,-2},{1,1},{4,-1},
                        {2,1},{-2,1},{5,-1},{1,-2},{3,1},{0,-3},{-1,-1},{6,-1} };
    printf("        o que se COLHE do doador          o que se DERIVA, sem lhe perguntar\n");
    printf("        %-33s %s\n", "os termos (o lado branco)", "ν(a,b) = (a + B·b, −b)");
    printf("        %-33s %s\n", "", "⊗ o produto, pela borda");
    printf("        %-33s %s\n", "", "N(a,b) = a² + B·ab + C·b²");
    printf("        %-33s %s\n\n", "", "Δ, e com ele o regime");

    int derivou = 0, falhou = 0;
    for(int d = 0; d < D; d++){
        long x[8]; x[0] = 0; x[1] = 1;
        for(int k = 2; k < 8; k++) x[k] = reg[d][0]*x[k-1] - reg[d][1]*x[k-2];
        Regua r = regua_de(x, COLHE);
        if(!r.fechou){ falhou++; continue; }
        /* o lado NEGRO, derivado — e verificado em 121 pontos, sem tocar no doador */
        int mau = 0;
        for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
            long va = a + r.B*b, vb = -b;
            long va2 = va + r.B*vb, vb2 = -vb;
            if(va2 != a || vb2 != b) mau++;
            long na = a*va - r.C*b*vb, nb = a*vb + b*va + r.B*b*vb;
            if(na != a*a + r.B*a*b + r.C*b*b || nb != 0) mau++;
        }
        if(mau) falhou++; else derivou++;
    }
    printf("        %d das %d dimensões: o lado negro derivou e fechou em 121 pontos cada\n",
           derivou, D);
    ok("a outra metade não veio do doador — saiu da régua, com resíduo 0", derivou == D && !falhou);

    /* E A POUPANÇA MEDE-SE: sem a dualidade seria preciso colher os dois lados. */
    long com_dual = (long)D * COLHE;
    long sem_dual = (long)D * COLHE * 2;
    printf("        colhido COM dualidade   %ld números\n", com_dual);
    printf("        seria SEM ela           %ld números   (os dois lados, do doador)\n", sem_dual);
    ok("a dualidade poupa metade da colheita — e é exatamente metade, não 'cerca de'",
       sem_dual == 2*com_dual);

    conclui("é isto que faz da transfusão uma transfusão: metade atravessa, e a outra metade nasce cá.");
}

/* ================================================================================ */
/* §X5 — a conta refeita                                                            */
/* ================================================================================ */
static void secao_X5(void){
    printf("\n§X5  A CONTA REFEITA: o que fica no banco, contra os 159 GB que temos\n\n");

    /* Um embedding real tem 768 dimensões (o nomic-embed-text do semantico.c). A colheita é de
     * n+2 = 4 números por dimensão, e cada número é uma palavra do banco: 16 bytes (o par). */
    const long DIM = 768, NMAIS2 = 4, SLOT = 16;
    long por_corpo = DIM * NMAIS2 * SLOT;
    printf("        um corpo transfundido:\n");
    { char kb[24]; rt_escreve_decimal(1, por_corpo, 1024, 1, kb, sizeof kb);
      printf("           %ld dimensões × %ld termos × %ld bytes = %ld bytes  (%s KB)\n",
             DIM, NMAIS2, SLOT, por_corpo, kb); }

    /* 159 GB são 159·1024³ bytes — um inteiro, e cabe folgado no long (1,7e11). Quantos
     * corpos lá cabem é uma divisão de inteiros, e a pergunta «cabem mais de um milhão?»
     * responde-se sem a fazer: livre > 1000000·por_corpo. */
    const long livre = 159L * 1024 * 1024 * 1024;
    long quantos = livre / por_corpo;
    printf("\n        nos 159 GB livres da Patria cabem %ld corpos desses\n", quantos);
    printf("        (o doador de 314 B cabe UMA vez, e deixa 2 GB — mas não corre)\n");

    ok("um corpo transfundido cabe em dezenas de KB, não em GB", por_corpo < 100L*1024);
    ok("e cabem milhões deles no que temos — o espaço deixou de ser a pergunta, e a conta"
       " e' de inteiros: 159.1024^3 bytes contra um milhao de corpos, sem dividir",
       livre > 1000000L * por_corpo);

    /* E A COMPARAÇÃO QUE FECHA: a razão entre hospedar e transfundir. */
    /* 314 mil milhões de parâmetros a meio byte cada: 157·1024³ bytes, um inteiro. E «mais
     * de seis ordens de grandeza» é hospedar > 1000000·por_corpo, sem dividir. */
    const long hospedar = 157L * 1024 * 1024 * 1024;         /* 314 B params em 4 bits */
    printf("\n        hospedar o 314 B (4 bits)   %ld bytes\n", hospedar);
    printf("        transfundir um corpo        %ld bytes\n", por_corpo);
    printf("        a razão                     %ld × (exacta, %ld/%ld)\n",
           hospedar / por_corpo, hospedar, por_corpo);
    ok("a diferença é de mais de seis ordens de grandeza — e é por isso que a pergunta muda."
       " A conta e' de inteiros: 157.1024^3 bytes contra um milhao de corpos, sem dividir",
       hospedar > 1000000L * por_corpo);

    printf("\n     O QUE ISTO NÃO DIZ, e tem de ser dito: 768×4 números captam o CORPO — a régua,\n");
    printf("     o dual, o regime. Não captam o que o doador SABE. A transfusão leva a estrutura\n");
    printf("     que a interação revelou, e o quanto ela revela depende da interação, não da conta.\n");
    printf("     Medir isso exige o doador acordado, e é o passo seguinte: veste.sh + colhe_emb.sh.\n");

    conclui("o espaço deixou de ser a pergunta; a pergunta passou a ser quanto a interação revela.");
}

/* ================================================================================ */
int main(void){
    puts("transfusao.c — NÃO SE HOSPEDA O DOADOR: COLHE-SE O CORPO");
    puts("========================================================");
    puts("");
    puts("  O Aarão perguntou pela TRANSFUSÃO e eu fiz a conta de HOSPEDAR — quantos GB de pesos,");
    puts("  quanta RAM. Essa conta está certa e responde a outra pergunta. A transfusão não copia");
    puts("  o doador: colhe o mínimo, e completa o resto pela dualidade.");

    secao_X1(); secao_X2(); secao_X3(); secao_X4(); secao_X5();

    printf("\n========================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  A pergunta 'cabem 300 GB?' era a pergunta errada. Um corpo transfundido cabe em");
        puts("  48 KB, e no que temos cabem milhões. O que resta medir não é espaço — é quanto");
        puts("  a interação com o doador acordado revela da estrutura dele. E isso precisa do");
        puts("  doador a responder, não a ser lido.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
