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

---

## 21/08/2026 — a causa mecânica, e a regra que eu quebrei a tentar vê-la

**A assinatura não segue as dependências.** O `banco/erg.c` LÊ o enum de opcodes do
`banco/sql.c` e verifica que o montador concorda. O `sql.c` mudou (opcodes de 16 bits
entraram); a assinatura do `erg.c` **não mudou**; o atestado ficou `0`; e a bateria
nunca mais o correu. Resultado: *«o enum do sql.c tem 16 opcodes; este montador expõe 17
ao piloto — DISCORDAM em 3, o primeiro é TROCA»*, invisível.

Ao limpar e re-derivar tudo: **529 : 518 verdes, 11 falhas** — onde a corrida normal
dizia `529 : 529, 0 falhas`. Os 11 têm a assinatura **inalterada** e o resultado mudou de
`0` para `1`: `erg`, `fita`, `indexa_orbitas`, e oito `.js` (`avalia_macros`,
`corpo_disco`, `corpo_front`, `design_no_pdf`, `dois_streams`, `escala`, `isa_dupla`,
`sem_chute`).

**Um medidor que lê OUTRO ficheiro tem a sua verdade fora da sua assinatura.** É a
família toda: qualquer medidor cuja entrada não esteja no seu próprio texto — outro `.c`,
um `.tex`, um ficheiro em `/tmp`, o ambiente — fica verde para sempre depois do primeiro
atestado.

**E a maneira certa de forçar a re-derivação é `./tools/bateria.sh --refaz`**, que reabre
TODAS sem apagar atestado nenhum. Eu fiz `: > tools/atestados.txt`. O cabeçalho do
próprio ficheiro proíbe isso em voz alta — *«o atestado NUNCA se apaga: apagar atestado é
destruir fato, não refazer o teste»* — e regista que foi um flag `--tudo`, que truncava a
tabela, que **destruiu os atestados de 30/07/2026**. Desta vez nada se perdeu (529 nomes
antes e depois, só 11 resultados a mudar), mas isso foi sorte: um medidor que não
corresse — falta de `node`, timeout — perdia a linha e o facto. **Ler o cabeçalho da
ferramenta antes de a usar contra o seu próprio aviso.** Ver
[[feedback-destruir-antes-do-inventario]] e [[feedback-o-disco-limpo]].

Mesma família de [[feedback-assercoes-vazias]] e da bateria cega em
[[project-checkpoint-2026-08-04-a-separacao]]: **o portão a dizer verde sobre uma coisa
que já não existe.**
