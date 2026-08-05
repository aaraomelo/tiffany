---
name: feedback-o-numero-que-nao-cabe
description: "O teste mais barato contra um número escrito à mão — perguntar se ele CABE no tipo. A \"máquina de 80 bits\" era um uint64_t."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-05T04:03:14.933Z
---

**O número anotado à mão pode ser IMPOSSÍVEL, e o tipo denuncia-o de graça.**

`broca-so/ula/koch_memoria_exp.c` imprimia, no mesmo bloco `fprintf` e a **três linhas
uma da outra**:

```json
"delta_endereco_bits_vizinho": 15.7,
"nota": "endereco salta (~80 bits); colisoes guardam c2 distinto"
```

Media 15,7 e escrevia 80. E o endereço é `uint64_t` (`koch_atlas.h:27`) — **uma
distância de Hamming em 64 bits não passa de 64.** O número não estava só errado: não
cabia. O `KOCH_MEMORIA.md` propagava-o em dois sítios, e o projecto inteiro chamava-lhe
*«a máquina de 80 bits»*.

**Why:** os meus testes contra números escritos à mão eram todos caros — mutar a
entrada, comparar dois caminhos, derivar a referência. Este é grátis e corre antes de
tudo: **o valor cabe no tipo que o guarda?** Bits contra a largura da palavra, contagens
contra o tamanho do conjunto, probabilidades contra 1, percentagens contra 100. Um
número que não cabe nunca foi medido — é sempre cópia.

**How to apply:** ao ler qualquer magnitude anotada num comentário, numa nota de JSON ou
num `.md`, procurar o **tipo do que ela mede** antes de acreditar. Se for Hamming, ver a
largura; se for contagem, ver o cardinal; se for fração, ver o denominador. E a nota
passa a sair da variável (`dend_med`), não da cabeça — teste da mutação em
[[feedback-a-referencia-escrita-a-mao]].

**E a leitura certa costuma ser MELHOR que a inventada.** «~80 bits» só dizia *muito, e
para cima*. O 15,7 de 64 diz uma coisa: um hash uniforme daria **32** (metade), e estar
**abaixo** disso significa que o recipiente **retém vizinhança** — não é hash, são ~4
níveis de 4 bits a mudar num endereço de 16 níveis. O número medido tinha conteúdo; o
anotado não tinha nenhum.

Mesma família que [[feedback-assercoes-vazias]] (anotar em vez de corrigir) e
[[feedback-normalizar-nao-e-medir]]. A tese sobreviveu inteira — só a magnitude é que
era literária.
