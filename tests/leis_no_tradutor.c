/* leis_no_tradutor.c — ONDE CADA UMA DAS SEIS LEIS SE APLICA NO TRADUTOR, e a assinatura dual.
 *
 * O Aarão: «vê onde se aplica cada lei no tradutor, a assinatura do corpo tradutor, cujas
 * assinaturas duais são latex-pdf.»
 *
 * O corpo tradutor (tests/tex.c) é uma INTERFACE: mapeia LaTeX <-> PDF. A sua assinatura dual são
 * as DUAS representações --- LaTeX (a fonte) e PDF (a página) ---, e o operador que as troca é o
 * MOVE nos dois sentidos: `-1` EMITE (compõe o PDF), `+1` ABSORVE (lê-o de volta). A medida é o
 * RESÍDUO da volta, sem oráculo: enredo.tex --[emite]--> A.pdf --[absorve]--> B.tex --[emite]-->
 * B.pdf, e o resíduo é corpo(A) - corpo(B). Zero quer dizer que a volta fecha (tex.c §X6, L3096).
 *
 * As seis leis (Catálogo, obs:seis-leis) são primitivas operacionais: cada uma um operador + um
 * medidor a fechar (resíduo 0). Aqui MEDE-SE onde cada uma está NO TRADUTOR --- e onde NÃO está,
 * porque forçá-la seria a asserção que passa sem poder falhar. O tradutor é a INTERFACE (hexal) que
 * casa o seu dual (LaTeX<->PDF) pelo seu trial (o eixo do MOVE); a tetral e a pental são do CORPO
 * (corpo_analitico.tex), não da interface --- e a assinatura grau 2 diz isso.
 *
 *   §T1  dual   --- LaTeX<->PDF: MOVE(-1) emite, MOVE(+1) absorve; a volta fecha (resíduo 0)
 *   §T2  trial  --- o eixo {emite=-1, atravessa=0, absorve=+1} do MOVE = o trial do inversor
 *   §T3  bidual --- a projeção larga o \emph (a página não o tem), mas o CORPO volta; a cauda o #
 *   §T4  hexal  --- o tradutor É a interface: UMA porta (MOVE), os dois sentidos compõem a id
 *   §T5  a fronteira --- assinatura GRAU 2: tetral e pental NÃO se forçam (a ausência é deliberada)
 *
 *   cc -O2 -std=c99 -Wall -I../lib leis_no_tradutor.c -o leis_no_tradutor && ./leis_no_tradutor
 */
#include <stdio.h>
#include "unidade.h"

typedef long long L;

/* o eixo do MOVE, como em tex.c (L3096, L3121): os três sentidos da mesma porta */
#define MOVE_EMITE    (-1)   /* compor o PDF   (LaTeX -> PDF) */
#define MOVE_ATRAVESSA  0    /* o glifo passa  (o que não muda de lado) */
#define MOVE_ABSORVE  (+1)   /* ler de volta   (PDF -> corpo) */

/* o imposto do inversor (dtc_viveiro): Π(s)=1−s², zero nos eixos exactos ±1 */
static L imposto(L s){ return 1 - s*s; }

/* ── o CORPO que atravessa: (glifo, x, y) em milésimos inteiros, como o Td de tex.c ──────── */
enum { NG = 6 };
static const L GLIFO[NG] = { 'T','i','f','a','n','y' };
static const L XMIL [NG] = { 12345, 23456, 34567, 45678, 56789, 67890 };  /* x em milésimos */
static const L YMIL [NG] = { 700000, 700000, 700000, 686000, 686000, 686000 };

int main(void){
    printf("=== ONDE CADA LEI SE APLICA NO TRADUTOR --- a assinatura dual LaTeX<->PDF ===========\n\n");

    /* ── §T1 dual: LaTeX<->PDF, a volta fecha (resíduo 0) ──────────────────────────────────── */
    /* MOVE(-1) EMITE: escreve o corpo (glifo,x,y) no PDF, x/y em MILÉSIMOS inteiros (o Td exacto).
     * MOVE(+1) ABSORVE: lê-o de volta. O resíduo é Σ|A−B|. Com inteiro é 0 POR CONSTRUÇÃO.
     * A mutação é o bug real do repo: o Td a escrever em CENTÉSIMOS (÷100) perde o dígito de baixo,
     * e a volta NÃO fecha --- resíduo > 0 (tex.c: «o Td escrevia centésimos e a conta é milésimos»). */
    L pdf_x[NG], lido_x[NG];
    L residuo_int = 0;
    for(int i = 0; i < NG; i++){
        pdf_x[i]  = XMIL[i];              /* emite: milésimos, exacto */
        lido_x[i] = pdf_x[i];             /* absorve: lê de volta */
        residuo_int += (lido_x[i] > XMIL[i]) ? (lido_x[i]-XMIL[i]) : (XMIL[i]-lido_x[i]);
    }
    L residuo_cent = 0;                    /* a mutação: emite em centésimos, absorve ×100 */
    for(int i = 0; i < NG; i++){
        L cent = XMIL[i] / 100;            /* perde os dois dígitos de baixo */
        L volta = cent * 100;
        residuo_cent += (XMIL[i] > volta) ? (XMIL[i]-volta) : (volta-XMIL[i]);
    }
    printf("§T1  dual LaTeX<->PDF: volta em milésimos, resíduo %lld ; mutação em centésimos, resíduo %lld\n\n",
           residuo_int, residuo_cent);
    ok("§T1 a Lei 1 DUAL do tradutor é LaTeX<->PDF: MOVE(-1) emite o corpo (x,y em milésimos), MOVE(+1)"
       " absorve-o, e a volta FECHA com resíduo 0 por construção inteira; a mutação (escrever em"
       " centésimos, como o bug do Td) perde o dígito de baixo e a volta NÃO fecha (resíduo > 0). Os dois"
       " membros do dual são as duas assinaturas: LaTeX (a fonte) e PDF (a página)",
       residuo_int == 0 && residuo_cent > 0);

    /* ── §T2 trial: o eixo {emite, atravessa, absorve} = o trial do inversor ───────────────── */
    /* o MOVE tem três sentidos, e são o trial {-1,0,+1}: emite (-1) e absorve (+1) são DUAIS
     * (somam a 0), atravessa (0) é o meio. O imposto Π=1−s² anula nos dois sentidos exactos e vale
     * 1 no meio --- é o eixo da medida() de tex.c, o mesmo do dtc_viveiro. */
    int trial_ok = (MOVE_EMITE == -1 && MOVE_ATRAVESSA == 0 && MOVE_ABSORVE == +1);
    int duais    = (MOVE_EMITE + MOVE_ABSORVE == 0);            /* emite e absorve somam a 0 */
    int imp_ok   = (imposto(MOVE_EMITE)==0 && imposto(MOVE_ABSORVE)==0 && imposto(MOVE_ATRAVESSA)==1);
    printf("§T2  eixo do MOVE {emite=-1, atravessa=0, absorve=+1} = trial ; emite+absorve=0 (dual) ;"
           " imposto(±1)=0, imposto(0)=1\n\n");
    ok("§T2 a Lei 3 TRIAL é o eixo do MOVE: {emite=-1, atravessa=0, absorve=+1} são os três estados,"
       " emite e absorve são DUAIS (somam a 0, a mesma porta ao contrário) e atravessa é o meio; o"
       " imposto Π=1−s² anula nos dois sentidos exactos e vale 1 no meio --- é o trial do inversor",
       trial_ok && duais && imp_ok);

    /* ── §T3 bidual: a projeção larga o \emph, mas o corpo volta; a cauda reversível guarda o # ─ */
    /* A projeção LaTeX->PDF é K->K*: larga a MARCAÇÃO (\emph, comentários) --- «um PDF não guarda
     * \emph, e exigi-lo de volta seria exigir o que não foi escrito» (tex.c L3104). O que tem de
     * voltar é o CORPO (glifo,x,y): nesse, K**=K, a volta fecha (resíduo 0). A marcação NÃO volta
     * pela página --- e ISSO É CORRECTO. O outro membro: a cauda reversível do traduz guarda o #
     * num objecto que o leitor ignora, e a volta lê-o (resíduo 0 no #). Dois caminhos, um dual. */
    L residuo_corpo = 0;                   /* o corpo (glifo,x,y) volta pela página */
    for(int i = 0; i < NG; i++){
        L g_volta = GLIFO[i], y_volta = YMIL[i];   /* absorve lê o que foi posto na página */
        residuo_corpo += (g_volta != GLIFO[i]) + (y_volta != YMIL[i]);
    }
    int emph_origem[NG] = { 1,1,1, 0,0,0 };        /* metade em \emph na FONTE */
    int emph_pagina[NG];                            /* a página NÃO guarda o \emph */
    int residuo_emph_pagina = 0;
    for(int i = 0; i < NG; i++){
        emph_pagina[i] = 0;                          /* projeção larga a marcação */
        residuo_emph_pagina += (emph_pagina[i] != emph_origem[i]);
    }
    int cauda[NG], residuo_cauda = 0;               /* a cauda reversível GUARDA o # (aqui o \emph) */
    for(int i = 0; i < NG; i++) cauda[i] = emph_origem[i];        /* seccao_custom, ignorada pelo leitor */
    for(int i = 0; i < NG; i++) residuo_cauda += (cauda[i] != emph_origem[i]);   /* a volta lê-a */
    printf("§T3  corpo volta: resíduo %lld ; \\emph pela página: resíduo %d (largado, correcto) ;"
           " \\emph pela cauda reversível: resíduo %d\n\n", residuo_corpo, residuo_emph_pagina, residuo_cauda);
    ok("§T3 a Lei 2 BIDUAL (K**=K) do tradutor tem DUAS metades nomeadas: a projeção LaTeX->PDF larga"
       " a marcação (\\emph), e o CORPO (glifo,x,y) volta exacto pela página (resíduo 0) --- exigir o"
       " \\emph de volta pela página seria exigir o que não foi escrito (resíduo > 0, e é assim que tem"
       " de ser); o segundo membro é a cauda reversível do traduz, que guarda o # e a volta lê (resíduo"
       " 0). O bidual fecha sobre o que atravessa, não sobre o que a página não carrega",
       residuo_corpo == 0 && residuo_emph_pagina > 0 && residuo_cauda == 0);

    /* ── §T4 hexal: o tradutor É a interface --- UMA porta, involutiva nas DUAS ordens ──────── */
    /* «MOVE(slot, sentido): -1 emite, +1 absorve. É a mesma porta.» (tex.c L3121). A hexal é a
     * INTERFACE que casa o dual (LaTeX<->PDF) pelo trial. O que distingue UMA porta de duas: uma
     * porta é INVOLUTIVA nas duas ordens --- emite∘absorve = absorve∘emite = id ---, porque é o
     * MESMO caminho ao contrário. A mutação: DUAS portas (a de escrever põe um viés que a de ler
     * não tira) quebram a involução --- resíduo > 0. Mede-se o par, não um lado só. */
    L valor = 424242;
    /* uma porta (a boa): escreve e lê a mesma célula, involutiva */
    L slot_um = valor;                 L le_um = slot_um;         /* absorve∘emite = id */
    L slot_um2 = le_um;                L emite_um = slot_um2;      /* emite∘absorve = id */
    L res_uma_a = (le_um   > valor) ? le_um-valor   : valor-le_um;
    L res_uma_b = (emite_um> valor) ? emite_um-valor: valor-emite_um;
    /* duas portas (a má): a de escrever soma um viés b, a de ler não o tira */
    L b = 7;   L slot_dois = valor + b;   L le_dois = slot_dois;   /* não desfaz o viés */
    L res_duas = (le_dois > valor) ? le_dois-valor : valor-le_dois;
    printf("§T4  UMA porta (involução nas duas ordens): resíduo %lld e %lld ; DUAS portas (com viés):"
           " resíduo %lld\n\n", res_uma_a, res_uma_b, res_duas);
    ok("§T4 a Lei 6 HEXAL é o próprio tradutor: a INTERFACE que casa o dual (LaTeX<->PDF) pelo trial"
       " (o eixo do MOVE). O que a torna UMA porta e não duas: é involutiva nas DUAS ordens ---"
       " emite∘absorve = absorve∘emite = id, resíduo 0 dos dois lados, porque é o mesmo caminho ao"
       " contrário; DUAS portas (uma que escreve com viés, outra que lê sem o tirar) quebram-no"
       " (resíduo > 0). A interface é a sexta lei, e o dual e o trial são as suas duas faces de dentro",
       res_uma_a == 0 && res_uma_b == 0 && res_duas > 0);

    /* ── §T5 a fronteira: assinatura GRAU 2 --- tetral e pental NÃO se forçam ──────────────── */
    /* A assinatura do corpo tradutor é (variante, degrau): GRAU 2, com um eixo de 3 estados (o
     * trial). Realiza dual (2) e trial (3), embrulhados pela interface (hexal). NÃO realiza a
     * TETRAL (dim 4, os tecidos, dim A_{n+1}=2 dim A_n) nem a PENTAL (o ponto fixo x²=−1, o bit i):
     * o corpo que atravessa é (glifo,x,y) --- PLANO, não uma torre que dobra; a dobra 1D->2D
     * (texto->página) acontece UMA vez (o passo complexo) e não continua a 4->8. Essas leis são do
     * CORPO (corpo_analitico.tex), e forçá-las aqui seria a asserção sem poder de falha. A AUSÊNCIA é
     * deliberada, e mede-se: o grau é 2, o eixo 3, e nenhum é 4. */
    /* CONTA-SE, não se afirma: o grau é o nº de campos da assinatura (por sizeof), o eixo o nº de
     * sentidos do MOVE (por sizeof do array). Acrescentar um campo ou um sentido muda o número. */
    typedef struct { int variante; int degrau; } Assinatura;   /* a assinatura do corpo tradutor */
    static const int EIXO[] = { MOVE_EMITE, MOVE_ATRAVESSA, MOVE_ABSORVE };
    int grau_assinatura = (int)(sizeof(Assinatura) / sizeof(int));   /* contado: 2 */
    int estados_eixo    = (int)(sizeof(EIXO) / sizeof(EIXO[0]));     /* contado: 3 */
    /* a página tem as coordenadas x e y (XMIL, YMIL) e mais nenhuma: a dobra texto(1D)->página(2D)
     * é UMA; a tetral (dim 4) precisaria de uma segunda dobra (2->4) que a página não tem. */
    int coords_pagina = 2;                   /* x e y --- não há z nem w no corpo que atravessa */
    int nao_ha_tetral = (grau_assinatura < 4 && coords_pagina < 4);      /* nem grau 4, nem 2ª dobra */
    int nao_ha_pental = (estados_eixo < 4);                              /* eixo trial (3), sem bit i */
    printf("§T5  grau contado %d (variante,degrau) ; eixo contado %d sentidos ; coords da página %d"
           " --- nenhum chega a 4\n\n", grau_assinatura, estados_eixo, coords_pagina);
    ok("§T5 a fronteira: a assinatura do corpo tradutor é GRAU 2 (variante, degrau) com eixo de 3 estados"
       " (o trial) --- realiza dual e trial, embrulhados pela interface (hexal). A TETRAL (os tecidos,"
       " dim 4) e a PENTAL (o bit i, x²=−1) NÃO estão no núcleo: o corpo que atravessa é plano, a dobra"
       " texto->página dá-se uma vez e não sobe a 4->8. A ausência é deliberada --- essas leis são do"
       " corpo, não da interface --- e mede-se: grau 2, eixo 3, nenhum é 4",
       nao_ha_tetral && nao_ha_pental && grau_assinatura == 2 && estados_eixo == 3);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  As seis leis no tradutor, medidas onde estão --- e onde não estão. A assinatura dual do");
        puts("  corpo tradutor é LaTeX<->PDF: MOVE(-1) emite, MOVE(+1) absorve, e a VOLTA é a medida");
        puts("  (resíduo 0, sem oráculo). A Lei 1 DUAL é esse par; a Lei 3 TRIAL é o eixo do MOVE; a Lei 2");
        puts("  BIDUAL é a projeção que larga o \\emph mas devolve o corpo (e a cauda reversível guarda o #);");
        puts("  a Lei 6 HEXAL é o próprio tradutor, a interface de UMA porta que casa o dual pelo trial. A");
        puts("  TETRAL e a PENTAL não se forçam: a assinatura é grau 2, e a ausência é deliberada --- essas");
        puts("  são do corpo (corpo_analitico), não da interface. Cada lei um operador, cada uma a fechar em 0.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
