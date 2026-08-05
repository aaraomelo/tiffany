---
name: feedback-o-medidor-que-nunca-mediu
description: "QUATRO medidores diziam \"NAO MEDIU\" e a bateria contava-os como verdes — a atestação guardava o exit e ninguém o lia"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-05T14:19:06.436Z
---

**Um medidor que não mede pode passar meses a dizê-lo em voz alta sem ninguém ouvir.**

Quatro deles, na mesma bateria:

| medidor | dizia | atestado |
|---|---|---|
| `transfusao_real` | *«NÃO MEDIU — sem vetores do doador»* | `exit 2` |
| `dualcifra` | *«NÃO MEDIU — sem frases/palavras»* | `exit 2` |
| `protocolo` | *«NÃO MEDIU — corra ./protocolo.sh»* | `exit 2` |
| `encaixa` | *«sem /tmp/emb.txt — corre colhe_emb.sh»* | `exit 1` |

E a bateria dizia **288/288 verdes**. A tabela `tools/atestados.txt` guardava
`<nome> <assinatura> <exit>`, e um resultado atestado não voltava a ser questionado — o
número de saída ficava lá como *facto conhecido* em vez de *falha por resolver*.

**Why:** o mecanismo é bom (evita re-derivar o que já foi provado), mas a atestação
guarda o **resultado** e não o **motivo**. Um `exit 2` que significa «não tenho dados» é
indistinguível de um `exit 2` que significa «medi e está certo assim». Depois de
guardado, os dois passam.

**How to apply:** **nenhum dos quatro foi apanhado por uma asserção.** Foram todos
apanhados por a **assinatura do `.c` mudar** — a migração para o disco tocou nos
ficheiros, o atestado caiu, e a falha apareceu. Logo:

1. **Um medidor que não corre há muito é suspeito, não estável.** Se a assinatura nunca
   muda, o atestado nunca é revisto, e ele pode estar morto há um ano.
2. **Procurar na tabela por `exit != 0`** é o teste directo, e custa um `awk`. Foi assim
   que se chegou aos quatro (seis linhas, e duas eram órfãs).
3. **Órfãos também contam:** 12 atestações eram de medidores que já não existem. Não
   mascaram nada por si, mas são o rasto que leva aos que mascaram.
4. E o portão passa a **separar «não mediu» de «falhou»** — contar como falha é honesto,
   mas some num total, e um total não diz a ninguém que basta correr um script.

**E por baixo de um deles havia um defeito a sério**, que só apareceu depois de ele
medir: o `colhe` escrevia o **padrão de bits** do float em hex e o `.c` lia com `strtod`
— `0x3F0EB6A8` virava `1 057 424 552` em vez de `0,557`. As duas asserções que falharam
estavam **certas**: eram elas a dizer que o que entrava não eram embeddings. Ver
[[feedback-o-numero-que-nao-cabe]].

Mesma família de [[feedback-assercoes-vazias]] e da bateria cega em
[[project-checkpoint-2026-08-04-a-separacao]]: **o portão a dizer verde sobre uma coisa
que já não existe.**
