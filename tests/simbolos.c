/* simbolos.c — AS OITO RELAÇÕES, VARRIDAS INTEIRAS.
 *
 *   cc -O2 -std=c99 -Ilib -Itests -o /tmp/simbolos tests/simbolos.c && /tmp/simbolos
 *
 * O `fisica.tex §fis:simbolos` deriva os símbolos de relação de uma dobra sobre
 * S×S em vez de os postular. Este medidor não testa uma amostra: as relações são
 * oito, os pares delas sessenta e quatro — cabe tudo, e por isso não há aqui
 * tecto, profundidade nem semente.
 *
 * A UNIDADE É A AFIRMAÇÃO, E NÃO A ITERAÇÃO. As varreduras aqui dentro fazem
 * centenas de comparações; emitir uma unidade por comparação dobrava a bateria
 * com ruído e não dizia mais nada, porque uma varredura ou fecha inteira ou não
 * fecha. Cada `ok()` é uma CLÁUSULA do teorema, com a varredura dela dobrada
 * num booleano — o endereço é a frase, e a frase é o que se prova.
 *
 *   §S1  (8) são OITO, e o índice é o subconjunto lido em binário
 *   §S2  (3)(4) a árvore das perguntas concorda com a leitura directa
 *   §S3  (9) a complementação: involução, quatro pares, NENHUM fixo
 *   §S4  (9) a τ: involução, troca os dois lados, fixa as outras quatro
 *   §S5  (9) as duas COMUTAM, e o que geram é o quarteto
 *   §S6  a cisão quatro/quatro, e ela lê-se no CARDINAL
 *   §S7  (7) ≤ = < ⊔ = , e o que as uniões e intersecções fecham
 *   §S8  o par (op, nega) derivado reproduz os seis sinais do SQL
 *   §S9  a canonização por τ não muda a verdade
 *   §S10 o ALCANCE decide para as oito, e a tabela de três casos não
 *   §S11 a leitura e a escrita fecham uma na outra
 */
#include <string.h>
#include "unidade.h"
#include "simbolos.h"

/* A leitura DIRECTA de cada relação, escrita à mão a partir da tabela do
 * teorema. É contra ela que a árvore das perguntas se mede, e é de propósito
 * que não usa NADA do header: duas escritas da mesma coisa, e uma delas não
 * sabe como a outra está feita. */
static int directa(int m, long d){
    switch(m){
        case SB_NULA:  return 0;
        case SB_LT:    return d <  0;
        case SB_EQ:    return d == 0;
        case SB_LE:    return d <= 0;
        case SB_GT:    return d >  0;
        case SB_NE:    return d != 0;
        case SB_GE:    return d >= 0;
        default:       return 1;
    }
}

static const long D[] = { -7, -1, 0, 1, 7 };     /* um par por bloco, e sobra */
#define ND ((int)(sizeof D / sizeof D[0]))

int main(void){
    printf("\n=== AS OITO RELAÇÕES: três blocos, 2³, e nem uma a mais ===\n\n");

    /* ── §S1 ─────────────────────────────────────────────────────────────── */
    printf("  §S1  as oito, e o índice é o subconjunto\n");
    {   int visto[SB_N]; memset(visto, 0, sizeof visto);
        int todas[SB_N] = { SB_NULA, SB_LT, SB_EQ, SB_LE, SB_GT, SB_NE, SB_GE, SB_TOTAL };
        int cabem = 1;
        for(int i = 0; i < SB_N; i++){
            if(todas[i] < 0 || todas[i] >= SB_N){ cabem = 0; continue; }
            visto[todas[i]]++;
        }
        int cobrem = 1;
        for(int m = 0; m < SB_N; m++) if(visto[m] != 1) cobrem = 0;
        ok("as oito cabem em três bits e cobrem 0..7, sem buraco e sem repetição",
           cabem && cobrem);
        ok("e o índice É o subconjunto: ≤ toma < e =, e não toma >",
           (SB_LE & (1 << SB_B_LT)) && (SB_LE & (1 << SB_B_EQ)) && !(SB_LE & (1 << SB_B_GT)));
        conclui("sem a nula e a total seriam seis, e seis não é potência de dois");
    }

    /* ── §S2 ─────────────────────────────────────────────────────────────── */
    printf("\n  §S2  a árvore das perguntas\n");
    {   int bom = 1, n = 0;
        for(int m = 0; m < SB_N; m++)
            for(int i = 0; i < ND; i++, n++)
                if(sb_vale(m, D[i]) != directa(m, D[i])) bom = 0;
        ok("a árvore das perguntas concorda com a leitura directa nas oito", bom);
        printf("      %d pares (relação, diferença) varridos\n", n);
    }

    /* ── §S3 ─────────────────────────────────────────────────────────────── */
    printf("\n  §S3  a complementação\n");
    {   int invol = 1, nega = 1, pares = 0, fixos = 0;
        for(int m = 0; m < SB_N; m++){
            if(sb_compl(sb_compl(m)) != m) invol = 0;
            if(sb_compl(m) == m) fixos++;
            if(sb_compl(m) >  m) pares++;
            for(int i = 0; i < ND; i++)
                if(sb_vale(sb_compl(m), D[i]) != !sb_vale(m, D[i])) nega = 0;
        }
        ok("a complementação é involução nas oito", invol);
        ok("e complementar é negar, ponto a ponto", nega);
        ok("quatro pares, e NENHUM fixo — três blocos não se repartem ao meio",
           pares == 4 && fixos == 0);
        ok("os quatro pares são ∅↔total, =↔≠, <↔≥ e >↔≤",
           sb_compl(SB_NULA) == SB_TOTAL && sb_compl(SB_EQ) == SB_NE
           && sb_compl(SB_LT) == SB_GE  && sb_compl(SB_GT) == SB_LE);
        printf("      pares: %d   fixos: %d\n", pares, fixos);
    }

    /* ── §S4 ─────────────────────────────────────────────────────────────── */
    printf("\n  §S4  a transposição τ\n");
    {   int invol = 1, troca = 1, fixos = 0;
        for(int m = 0; m < SB_N; m++){
            if(sb_tau(sb_tau(m)) != m) invol = 0;
            if(sb_tau(m) == m) fixos++;
            /* τ na relação é τ no par: trocar (x,y) por (y,x) nega a diferença */
            for(int i = 0; i < ND; i++)
                if(sb_vale(sb_tau(m), D[i]) != sb_vale(m, -D[i])) troca = 0;
        }
        ok("τ é involução nas oito — a Def. do operador, e nada mais", invol);
        ok("e τ na relação é τ no par: ela nega a diferença", troca);
        ok("τ fixa exactamente quatro, e são ∅, =, ≠ e a total",
           fixos == 4 && sb_tau_fixa(SB_NULA) && sb_tau_fixa(SB_EQ)
           && sb_tau_fixa(SB_NE) && sb_tau_fixa(SB_TOTAL));
        ok("e troca < com > e ≤ com ≥",
           sb_tau(SB_LT) == SB_GT && sb_tau(SB_LE) == SB_GE);
        printf("      fixos de τ: %d\n", fixos);
    }

    /* ── §S5 ─────────────────────────────────────────────────────────────── */
    printf("\n  §S5  as duas dobras, e o quarteto\n");
    {   int comutam = 1, ordem2 = 1, distintas = 1;
        for(int m = 0; m < SB_N; m++){
            if(sb_compl(sb_tau(m)) != sb_tau(sb_compl(m))) comutam = 0;
            if(sb_compl(sb_compl(m)) != m
               || sb_tau(sb_tau(m)) != m
               || sb_compl(sb_tau(sb_compl(sb_tau(m)))) != m) ordem2 = 0;
            if(sb_tau(m) == sb_compl(m)) distintas = 0;
        }
        ok("as duas dobras comutam nas oito — agem em coisas distintas", comutam);
        ok("e os três não triviais têm ordem dois: é o quarteto, não o cíclico", ordem2);
        ok("τ e a complementação são aplicações distintas", distintas);
        conclui("uma mexe em QUAIS blocos; a outra em QUAL lado");
    }

    /* ── §S6 ─────────────────────────────────────────────────────────────── */
    printf("\n  §S6  a cisão, e ela é de cardinal\n");
    {   int por_tau = 1, por_cardinal = 1, rev = 0, ori = 0;
        for(int m = 0; m < SB_N; m++){
            if(sb_reversivel(m) != (sb_tau(m) == m)) por_tau = 0;
            if(sb_orientada(m) != (sb_pop(m & SB_LADOS) % 2 == 1)) por_cardinal = 0;
            if(sb_reversivel(m)) rev++; else ori++;
        }
        ok("reversível ⟺ a τ a fixa", por_tau);
        ok("e a leitura por CARDINAL dá o mesmo: par não orienta, ímpar orienta",
           por_cardinal);
        ok("quatro reversíveis (∅,=,≠,total) e quatro orientadas (<,>,≤,≥)",
           rev == 4 && ori == 4
           && sb_reversivel(SB_NULA) && sb_reversivel(SB_EQ)
           && sb_reversivel(SB_NE)   && sb_reversivel(SB_TOTAL)
           && sb_orientada(SB_LT) && sb_orientada(SB_GT)
           && sb_orientada(SB_LE) && sb_orientada(SB_GE));
        printf("      reversíveis (directo): %d   orientadas (cruzado): %d\n", rev, ori);
        conclui("contar não orienta — é preciso escolher, e escolher é um bit");
    }

    /* ── §S7 ─────────────────────────────────────────────────────────────── */
    printf("\n  §S7  o fecho é juntar o vinco\n");
    {   ok("≤ = < ⊔ = e ≥ = > ⊔ =, e as uniões são DISJUNTAS",
           (SB_LT | SB_EQ) == SB_LE && (SB_GT | SB_EQ) == SB_GE
           && (SB_LT & SB_EQ) == SB_NULA && (SB_GT & SB_EQ) == SB_NULA);
        ok("≤ e ≥ cobrem S×S e cruzam-se exactamente no vinco",
           (SB_LE | SB_GE) == SB_TOTAL && (SB_LE & SB_GE) == SB_EQ);
        ok("< e > cobrem o tecido e não se cruzam",
           (SB_LT | SB_GT) == SB_NE && (SB_LT & SB_GT) == SB_NULA);
        ok("«um lado», e não «o tecido»: o tecido mais o vinco é a total, e ≤ não é",
           (SB_NE | SB_EQ) == SB_TOTAL && SB_LE != SB_TOTAL);
    }

    /* ── §S8 ─────────────────────────────────────────────────────────────── */
    printf("\n  §S8  o par (op, nega) é DERIVADO\n");
    {   struct { int m; int op; int nega; const char *txt; } esperado[] = {
            { SB_EQ, '=', 0, "="  }, { SB_NE, '=', 1, "<>" },
            { SB_LT, '<', 0, "<"  }, { SB_GE, '<', 1, ">=" },
            { SB_GT, '>', 0, ">"  }, { SB_LE, '>', 1, "<=" },
        };
        int bate = 1;
        for(int i = 0; i < 6; i++){
            if(sb_op(esperado[i].m) != esperado[i].op)     bate = 0;
            if(sb_nega(esperado[i].m) != esperado[i].nega) bate = 0;
            printf("      %-2s  →  op '%c'  nega %d\n", esperado[i].txt,
                   sb_op(esperado[i].m), sb_nega(esperado[i].m));
        }
        ok("os seis sinais do SQL saem da derivação, e não de uma tabela ao lado", bate);
        ok("∅ e a total não têm op, e são exactamente as duas que dispensam a linha",
           sb_op(SB_NULA) == 0 && sb_op(SB_TOTAL) == 0
           && sb_decidida(SB_NULA) && sb_decidida(SB_TOTAL)
           && !sb_constante(SB_NULA) && sb_constante(SB_TOTAL));
        int equiv = 1;
        for(int m = 0; m < SB_N; m++) if(sb_decidida(m) != (sb_op(m) == 0)) equiv = 0;
        ok("sem op ⟺ decidida em compilação", equiv);
    }

    /* ── §S9 ─────────────────────────────────────────────────────────────── */
    printf("\n  §S9  a canonização por τ\n");
    {   int conserva = 1, ao_lado = 1, transpostas = 0, fixa_pede = 0;
        for(int m = 0; m < SB_N; m++){
            if(sb_tau_fixa(m) && sb_transpor_pede(m)) fixa_pede = 1;
            if(!sb_transpor_pede(m)) continue;
            transpostas++;
            for(int i = 0; i < ND; i++)
                if(sb_vale(m, D[i]) != sb_vale(sb_tau(m), -D[i])) conserva = 0;
            if(sb_op(sb_tau(m)) != '>') ao_lado = 0;
        }
        ok("transpor a relação E negar a diferença conserva a verdade", conserva);
        ok("e leva sempre para o lado escolhido, que é o `>`", ao_lado && transpostas == 2);
        ok("nenhuma τ-fixa pede transposição — para elas o par e o transposto são um facto",
           !fixa_pede);
        printf("      pedem τ: %d (< e ≥)   τ-fixas: 4\n", transpostas);
    }

    /* ── §S10 ────────────────────────────────────────────────────────────── */
    printf("\n  §S10  o alcance, contra a varredura exaustiva\n");
    {   long n = 0, err_alcance = 0, err_tres = 0;
        int nula_ok = 1, total_ok = 1;
        for(long long b = -4; b <= 4; b++)
        for(long long a = b; a <= 4; a++){
            if(sb_possivel(SB_NULA, b, a))  nula_ok  = 0;
            if(!sb_possivel(SB_TOTAL, b, a)) total_ok = 0;
            for(int m = 0; m < SB_N; m++, n++){
                int existe = 0;
                for(long long d = b; d <= a; d++)
                    if(sb_vale(m, (long)d)) { existe = 1; break; }
                if(sb_possivel(m, b, a) != existe) err_alcance++;
                /* e o que a tabela de três casos — a que olha o `op` derivado e
                 * ignora a `nega` — teria respondido no mesmo sítio. Ela dá o
                 * falso VAZIO nas complementares, que é descartar a consulta
                 * exactamente quando ela é verdadeira para todas as linhas. */
                {   int tres = 1;
                    switch(sb_op(m)){
                        case '>': if(a <= 0) tres = 0; break;
                        case '<': if(b >= 0) tres = 0; break;
                        case '=': if(a < 0 || b > 0) tres = 0; break;
                        default: break;
                    }
                    if(!tres && existe) err_tres++;
                }
            }
        }
        ok("o alcance concorda com a varredura exaustiva, ponto a ponto",
           err_alcance == 0);
        ok("e a tabela de três casos ERRA — é o defeito que isto fecha", err_tres > 0);
        ok("a nula nunca é possível; a total é sempre — sem olhar o intervalo",
           nula_ok && total_ok);
        printf("      %ld casos (intervalo × relação): o alcance errou %ld;"
               " a tabela de três casos deu %ld falsos VAZIOS\n", n, err_alcance, err_tres);
    }

    /* ── §S11 ────────────────────────────────────────────────────────────── */
    printf("\n  §S11  a leitura e a escrita\n");
    {   const char *sinais[] = { "=", "<", ">", "<=", ">=", "<>", "!=" };
        int consome = 1, volta = 1;
        for(int i = 0; i < 7; i++){
            int n = 0, m = sb_le(sinais[i], &n), n2 = 0;
            if(n != (int)strlen(sinais[i])) consome = 0;
            if(sb_le(sb_escreve(m), &n2) != m) volta = 0;
        }
        ok("consome o sinal inteiro, e escrever e reler dá a mesma relação",
           consome && volta);
        {   int a = 0, b = 0, c = 0;
            ok("`<>` e `!=` são a MESMA relação, e o `<>` lê-se de uma vez",
               sb_le("<>", &a) == sb_le("!=", &b) && sb_le("<>", &c) == SB_NE && c == 2);
        }
        {   int n1 = 0, n2 = 0;
            sb_le("abc", &n1); sb_le("", &n2);
            ok("onde não há sinal consome ZERO — e isso não é «um sinal que não conheço»",
               n1 == 0 && n2 == 0);
        }
    }

    printf("\n  %d unidades, %d falha(s)\n", unidades, falhas);
    if(!falhas) printf("  as oito fecham: 2³ e nem uma a mais.\n\n");
    return falhas ? 1 : 0;
}
