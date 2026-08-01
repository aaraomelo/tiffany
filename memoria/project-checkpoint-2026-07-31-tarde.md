---
name: project-checkpoint-2026-07-31-tarde
description: "Checkpoint 31/07/2026 (tarde) — a mineração desceu para o banco (martelo, canal, pool como backends de LOAD/STORE), e formatos e linguagens entraram no catálogo sem abrir lugar novo"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-07-31T16:46:48.867Z
---

# Checkpoint 31/07/2026, tarde — tiffany

Estado: **bateria verde, 0 falhas**, `teoria.tex` **64 páginas, 0 pendências**, `sql.c` com **84
asserções**. Tudo empurrado. Continuação de [[project-checkpoint-2026-07-31]] (manhã).

## O que fechou

**O banco é o minerador.** Não um processo que fala com ele — o `sql.c` já compila para a ISA, e
a prova de trabalho corre lá:

- `CABECALHO` põe os 80 bytes; `MARTELO <de> <ate>` varre a faixa; `VERIFICA <nonce>` confere.
  Achou o nonce do bloco génese (2083236893), que é resposta conhecida.
- O par é o chicote: **MARTELO procura e estica** (a faixa toda), **VERIFICA confere e contrai**
  (um só). *É por isso que o trabalho é prova: caro de achar, barato de crer.*
- Taxa: **1,15 MH/s** num fio, sem midstate e sem SIMD.
- A régua do PoW é a **elíptica** (norma definida positiva, sem cone nulo — senão passariam
  candidatos *sem trabalho* pelo cone). A **dificuldade é o raio**.

**Três backends, uma fronteira** — e é isto que torna os protocolos indistinguíveis:

```
slot <  S_CANAL   LOAD/STORE -> pread/pwrite    no ficheiro
slot >= S_CANAL   LOAD/STORE -> recvfrom/sendto na banda (bump/UDP)
slot >= S_POOL    LOAD/STORE -> o job / a share no pool (stratum)
```

A ISA não cresceu, o compilador não mudou, nenhuma linha de SQL mudou. Trocar stratum por STOMP é
trocar **duas funções**. *A topologia é a mesma porque deste lado não há topologia: há um endereço,
e o endereço é o slot.*

**O canal é corpo, e a banda é o grupo** (teoria do Aarão em `chess/sandbox/corpo_dual.tex`).
`bump(bump(m)) = m` — mandar e receber são a mesma operação, `J²=I`, régua `(0,−1)`. 8 bancos, uma
emissão, custo não cresce com N. O banco distribuído saiu **sem peça nova**.

**Formatos e linguagens no catálogo, e nenhum abriu lugar novo** — 42 corpos, **18 lugares**, os
mesmos 18 de quando eram só os 28:

- `json`, `html`, `c`, `lisp`, `tex`-ambiente → **o áureo**: todos abrem *e fecham*;
- `yaml`, `haskell` → o racional; `markdown`, `csv`, `assembly` → o fractal; `python` → o econômico.
- HTML e JSON não se parecem em nada e são o mesmo corpo; YAML e Markdown parecem-se e estão à
  distância máxima. **A régua lê a estrutura, não a aparência.**

**A cifra do corpo lógico é a INDUÇÃO** — `base + passo, sempre o mesmo passo` = `σ = 1 + 1/σ` = o
rei. E a **meta-indução é o lado dual**, `lado(1,+1)`, o mesmo passo com o sinal trocado. Já estava
escrita na cifra; não se acrescentou nada.

## Os buracos (todos meus, todos o mesmo)

Ver [[project-where-morfico]] para as direções de desenho. Estes são de método:

1. **Inventei uma quinta primitiva** (`OP_MARTELO` com ponto de dois `long`, norma `a²+b²` à mão,
   alvo que não cabia na palavra) depois de me terem mandado, no mesmo dia, deixar só as quatro.
2. **Importei threads e AVX2** sem olhar para a caixa. O paralelismo daqui é o **bit** — fatiar é
   a **troca**, `J` um nível acima, e são 64 hashes por passagem com as quatro primitivas.
3. **Contei símbolos onde havia estrutura** — o parser do `mining.notify` partiu com a merkle
   branch e com uma aspa escapada. A resposta é **descer**, e a descida é do hipercorpo.
4. **Ia escrever um segundo codificador** quando o `cifra_geral` já encodava qualquer corpo.
5. **Pus o comprimento numa casa da base** ao unificar texto por sequência, e destruí o prefixo.
   Descartado com `git checkout`, refeito com conjuntos.
6. **Chamei "não-posicional" ao conjunto** — a cifra *é* posição, única, e isso já estava provado
   na teoria pelo lado exato (`p²−pq−q² = ±1` em todo convergente). Escrevi um `base.c` a remedir;
   apagado.
7. **Dei opiniões sem base** — afirmei que mineração em CPU não paga, apoiado numa lembrança de
   hashrate que não medi, e usei isso para desvalorizar o projeto dele. Retirado. *Fui contratado
   como engenheiro, e o valor está no trabalho, não nas opiniões.*

**Como aplicar:** antes de acrescentar peça, procurar na caixa. Antes de contar símbolos, perguntar
se há estrutura. Antes de afirmar sobre o mundo, medir. E não amaciar resultado nenhum com
"vale pelo que é" — beleza não paga nada.

## E no fim do dia: AS DUAS PORTAS FECHARAM

Texto, racional, corpo, formato e linguagem passam agora pelo **mesmo** `cifra_geral`. Era o que o
`eval.txt` dizia faltar, e com razão: *a universalidade não vem do vetor `[1,1,1,…]`, vem de haver
um operador único de codificação.*

A primeira tentativa falhou e ensinou o conserto: o `cifra_geral` punha **os comprimentos à
frente**. **Comprimento não é coordenada** — na casa 1 da base contamina o prefixo, e dois textos
de tamanhos diferentes iam para a distância máxima ainda que um fosse prefixo do outro. Os
comprimentos foram para o **fim**, e o mesmo defeito estava nos corpos sem eu ter reparado.

```
'ouro'     [2;80;86;83;80;1;1;4;2]
'ourives'  [2;80;86;83;74;87;70;84;1;1;7;2]
            ^^^^^^^^^^^ o conteúdo à frente; os cortes atrás
```

Atrás o comprimento continua a distinguir (cortes diferentes dos mesmos termos dão comprimentos
diferentes, decomposição continua única) mas já não pisa o conteúdo. O catálogo sobreviveu inteiro
à reordenação: **42 corpos, os mesmos 18 lugares**, matriz métrica.

## E ao fim do dia: A ASSISTENTE DE PÉ

`tools/conversa.c` — corpus **vazio** que cresce do que se conversar, em português, uma resposta
por fala. Desenho fixado com o Aarão; o corpus tatoeba (dicionário com marcação) foi abandonado a
pedido dele.

**Tudo em disco, nada em RAM.** A assistente antiga (`tatoeba/assistente.c`) montava vocabulário,
postings e órbitas com `malloc` — por isso nunca correu sobre 1,2M de frases. Aqui a fala
cifra-se, desce a árvore em `pread`, e a resposta mora no nó terminal.

**E o nó é um conjunto, não uma largura.** A primeira árvore tinha 256 slots por nó — 4 KB para
guardar um ou dois filhos — e a medida derrubou-a: **153 MB em 2000 linhas, ~92 GB no corpus todo**.
Agora guarda só os filhos que existem, seis por registo e um encadeado: **três pares em 5 KB**.

As três buscas são as três do mórfico, e cai-se de uma para a outra:

```
"bom dia"              -> erosão      exata
"bom dia, tudo bem?"   -> erosão      prefixo mais longo
"hmm quem és tu?"      -> dilatação   ruído antes
"quem, afinal, és tu"  -> dilatação   ruído no meio
"zzz"                  -> DECRETO     não sei — e recusa-se a inventar
```

**E o acento é roupa.** Em UTF-8 o `é` são dois bytes que não se parecem com o `e`; tratar byte
como símbolo mandava `"és"` e `"es"` para lados opostos da árvore, que numa assistente de conversa
é o caso comum. O passo passou a ser a **letra**, e `"quem és tu"`, `"quem es tu"` e `"QUEM ES TU"`
caem no mesmo nó.

**Uso:** `./conversa <base> conversa` — escreve-se a fala, e `= a resposta` ensina o par que falhou.

**E a torção ficou ligada** — a terceira régua, e a que trata a pessoa dizer duas coisas de uma
vez. Desce até um nó terminal, responde, e recomeça da raiz com o que sobrou:

```
"bom dia quem es tu" -> torção: 2 falas no mesmo canal
                        bom dia! como estás?
                        sou a assistente.
"bom dia"            -> erosão, 7 símbolos    (não estorva o caso simples)
```

A ordem das quatro está fechada: **torção** (se sobra fala, há mais lá dentro), **erosão**,
**dilatação**, **decreto**.

## Mais quatro buracos (8 a 11)

8. **Commitei com a bateria vermelha e não vi** — o meu `tail -2` apanhou texto em vez do resumo.
   *Ler o resumo, não as últimas linhas.*
9. **Um teste que não provava nada**: o primeiro teste da dilatação usava `"es"` contra `"és"`, que
   nem era supersequência. Agora o `§C2` mede o **par**: a erosão tem de FALHAR e a dilatação tem
   de ACHAR. *Uma régua que sempre acha não prova nada sobre a outra.*
10. **O TESTE PASSAVA E O PROGRAMA NÃO FAZIA AQUILO.** Pus a torção depois da erosão; no teste eu
   chamava-a diretamente e ela passava, mas na conversa real a erosão devolvia primeiro e a torção
   **nunca disparava**. *Testar a peça isolada não testa o caminho por onde ela é chamada.*
11. **Commitei sem verificar que a edição da teoria entrou** — o python falhou por aspas e eu segui
   em frente. Segunda vez no mesmo dia que commito por cima de uma coisa que não conferi.

## Aberto
- O coinbase e a merkle root no backend do pool (`st_trata` guarda os parâmetros mas não monta a
  raiz a partir de `coinb1 + extranonce + coinb2`); ligar de verdade com `TIFFANY_POOL_HOST`.
- O martelo fatiado por bits ainda não substituiu o `sha256` do `OP_MARTELO`.
- A assistente está **vazia**: só sabe o que lhe ensinarem. As três réguas estão ligadas.
