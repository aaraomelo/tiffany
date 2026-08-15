---
name: feedback-a-mensagem-que-nao-pode-falhar
description: "Escrevi `cc ... ; echo \"compilou\"` — o echo corria mesmo com a compilação falhada, ficou o binário velho, e reportei sucesso sobre nada."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T20:44:56.043Z
---

Corri `cc -O2 ... 2>&1 | grep error | head -3; echo "── compilou"`. A compilação **falhou**,
o `grep` mostrou o erro no meio da saída, e o `echo` imprimiu **«compilou»** logo a seguir.
Fiquei com o binário antigo e passei ao teste seguinte, que deu **0 de 21 falas** — só aí
percebi.

**Uma mensagem que não pode falhar é a mesma doença que uma asserção que não pode falhar**
([[feedback-assercoes-vazias]]). O `echo` não estava ligado a nada: era decoração com
forma de verificação.

## A forma certa

    if cc -O2 ... -o bin fonte.c 2> log; then echo "COMPILA"; else grep error log; fi

O `echo` passa a depender do **código de saída**, e a linha só aparece quando é verdade.

## O gatilho

Depois de qualquer comando que produz um **artefacto** (binário, PDF, ficheiro gerado),
perguntar: *se este comando falhar, a minha linha seguinte muda?* Se não muda, a linha
não mede nada — e pior, o artefacto **velho** continua lá e o teste seguinte corre sobre
ele. Foi exactamente isso: o binário anterior ainda respondia, e as falas novas
simplesmente não existiam.

Corolário: **um binário que não foi reconstruído é mais perigoso que um que não existe**,
porque o segundo dá erro e o primeiro dá respostas erradas em silêncio.

E na mesma hora, pela segunda vez na sessão: `Qz I = …` colidiu com a macro `I` do
`complex.h`. Nomes de uma letra em C não são livres —
[[feedback-duas-reguas]] na sua forma mais barata.

Ver [[feedback-o-medidor-que-nunca-mediu]], [[feedback-o-exit-sombreado]].
