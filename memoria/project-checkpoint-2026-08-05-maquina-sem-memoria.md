---
name: project-checkpoint-2026-08-05-maquina-sem-memoria
description: "Checkpoint 05/08: A Máquina Sem Memória — a RAM estática cai 85%, três medidores que nunca mediram, e o portão que estava cego"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-05T13:45:37.855Z
---

**O ciclo de engenharia começou, e o objecto é A MÁQUINA SEM MEMÓRIA** — a que lê e
escreve no disco e não guarda nada.

## O QUE ELA É, E PORQUE PODE SER

> **«Operar exige DUAS coordenadas ligadas por uma involução: uma que MEDE (aditiva, dá
> tamanho) e outra que ORDENA (multiplicativa, dá sentido e NÃO DÁ TAMANHO). Subir de
> dimensão não acrescenta tamanho: acrescenta FASE.»**

E o teorema que a tira do domínio do engenho: **toda representação tem dual e toda medida
tem dual, sem hipótese nenhuma sobre o objecto.** Daí:

| o teorema dá | a máquina ganha |
|---|---|
| toda representação tem dual | toda representação é **reversível** |
| reversível | desfaz-se **repetindo**, não lembrando |
| não precisa lembrar | **não precisa de memória** |

**A memória está no OBJECTO, não na máquina.** E a fase não se guarda: **lê-se** do par,
por Cassini — `det(A^k) = F_{k-1}F_{k+1} − F_k² = (−1)^k`. É a dualidade a ser a memória
da divisão. `broca-so/code/maquina.c`, 4 de 4, **zero símbolos em .bss** conferidos com
`nm` no próprio objecto.

## AS CORRECÇÕES DELE, POR ORDEM (cada uma desfez uma coisa minha)

1. **«o bit não é o determinante»** — o determinante é a **leitura** dele numa base. O bit
   não determina nada; ele É. Tudo o que precisa de base é realização.
2. **«1 bit PERCORRE todos os andares»** — não há um bit por andar. `dim A_n = 2^n` é a
   dimensão do OBJECTO. A torre é uma **trajectória**, não uma pilha.
3. **«64 bytes? pq isso não é negociável?»** — não era: era número escolhido, e a conta
   estava **errada por 33×** (deixei `h[N]` e `acc[N]` fora). O mínimo deriva-se de **quem
   depende de quê**.
4. **«compor é somar, põe uma ao lado da outra no slot»** — `A_n` é matriz, logo linear.
   Eu tinha concluído que compor obrigava a passar pela memória. Facto certo, conclusão
   falsa.
5. **«lê a teoria, pára de tentar»** — `thm:torrecruz`: *«Nenhum andar é construído; todos
   são lidos do anterior.»* Eu passei uma hora a construir andares.
6. **«o disco é lento baseado em nada»** — não medi. Medido: **1,1 M passos/s**, e **o
   cálculo é 0% do tempo**; 84% era abrir/fechar, meu. O ficheiro tem 16 bytes e vive no
   page cache — nunca tocou o prato.
7. **«não tem tempo em algo que é reversível»** — e a razão física: **Landauer**. Apagar
   um bit custa `kT ln2`; operação reversível não apaga, logo o mínimo é **zero**. A DRAM
   paga-o **em cada refresh, por existir**. É por isso que é cara, e não pelo preço/byte.

## A MIGRAÇÃO, MEDIDA

```
RAM estatica  69 153,8  ->  10 367,1 KB      -85%
              + 12 288 KB de ctl_, que sao CONTROLOS e ficam
```

- **`ula/disco.h`** — `#define buf DISCO_FIXO(T,k)` + `disco_prende`. mmap, o ficheiro É o
  vector. O **endereço é constante do programa, não variável** (um ponteiro global são 8
  bytes, e 8 não é 0). `DISCO_FIXO2` para 2D mantém `V[i][j]` intacto.
- **`tools/ram.sh`** — o portão: mede `.bss`/`.data` com `nm`, falha se subir, acusa quem
  não compila, e **poupa os `ctl_`**.

## OS TRÊS DEFEITOS ESTRUTURAIS QUE ISTO DESTAPOU

**1. O portão estava CEGO e escondia 38 MB.** Fazia `|| continue` e saltava os 287
ficheiros do tiffany em silêncio (falta `-Ilib`). Reportei 31 410 KB; a verdade eram
**69 153,8**. Mesmo defeito da bateria cega, cometido no portão escrito para o impedir.

**2. Há RAM que NÃO se migra.** `llm.c` e `mmu.c` têm 12 MB que são **controlos
deliberados** — *«alocados de propósito, para provar o custo que o disco evita»*.
Migrá-los cegava a asserção ao lado. Prefixo `ctl_`, e o portão poupa-os. **Sem isso o
portão pressionava na direcção de destruir a medição.**

**3. TRÊS MEDIDORES QUE NUNCA MEDIRAM**, tapados por atestação: `transfusao_real`,
`dualcifra`, `protocolo`. Os três pediam dados do doador (Ollama), os três diziam-no na
cara — *«NÃO MEDIU»* — e os três estavam atestados com `exit 2` e contados como verdes.
Colhidos, os três medem. E a `transfusao_real` tinha um defeito a sério por baixo:
**o `colhe` escrevia o PADRÃO DE BITS do float em hex e o `.c` lia com `strtod`** —
`0x3F0EB6A8` virava 1 057 424 552 em vez de 0,557. Daí erro relativo `0,000000` em todas
as escalas. **As duas asserções que falhavam estavam certas**: eram elas a dizer que o que
entrava não eram embeddings.

## O MEU PADRÃO DO DIA, e é UM só

**Comparar o que não é o resultado.** O migrador em lote reverteu ficheiros bons por
*avisos do compilador*, por *tempos de execução*, e por diferença nenhuma. E `sizeof` sobre
um ponteiro dá **8** — foi a causa comum de metade das reversões. Investigar **um a um**
encontra o que o lote esconde. Ver [[feedback-o-numero-que-nao-cabe]] e
[[feedback-assercoes-vazias]].

E três vezes **medi por código de saída sem pensar**: `grep -c "falha"` conta «0 falhas»;
`ls a b c` sai ≠ 0 se qualquer um faltar.

## O QUE FICA POR FAZER

1. **`protocolo.c`: 1 falha REAL e visível** — *«razões dentro de 0,05 de 1,0: 6 de 7»*, e
   a asserção exige todos. O doador é **estocástico**. Decidir: a asserção pede demais, ou
   aquela razão está mesmo fora? **Não mexer sem decidir** — mudar o limiar para caber no
   que saiu é anotar.
2. ~10 MB por migrar, todos < 400 KB (`cantor.c v2`, `semantico.c`, `encaixa.c`, …).
3. `transfusao_real` e `dualcifra` só medem com `/tmp/vetores.txt` — **os dados não estão
   versionados**, logo a bateria volta a não os medir depois de um reboot.

Ligado a [[project-checkpoint-2026-08-04-ciclo-fechado]] e [[feedback-nunca-usar-ram]].
