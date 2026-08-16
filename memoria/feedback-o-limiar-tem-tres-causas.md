---
name: feedback-o-limiar-tem-tres-causas
description: 918 limiares `1e-N` no repo, e 583 são pura DECORAÇÃO — sem transcendente na conta, a comparação podia ser por igualdade.
metadata:
  type: feedback
---

Auditado o repo: **3616 `double`** em 165 ficheiros (68 estrutura · 3157 realização ·
**350 medição** · 41 apresentação), e **918 limiares `1e-N`**. A triagem separa-os em três
causas, e **só uma é honesta**:

**(A) DECORAÇÃO — 583 dos 918.** Não há função transcendente na conta. O limiar não é
preciso e a comparação pode ser por **igualdade**: o resíduo é ZERO. Exemplo real: a lei
`A(f)/A(2f) = 4` media-se com `fabs(r−4) > 1e-9`, e o `c`, o `π` e as unidades
**cancelam-se** — o que sobra são dois quadrados de inteiros.
E o **pior caso da classe**: o limiar a dar cara de medição a uma **TAUTOLOGIA** — uma
matriz construída simétrica (`S[i][j] = S[j][i]` atribuído na linha acima), comparada com
`1e-14`. Não tolerava erro nenhum; escondia que era a construção a fechar.

**(B) SABOR.** Há transcendentes, mas a identidade é **algébrica** e vale para quaisquer
entradas — eles não entram na prova e só forçam o limiar. A adjunção `⟨Sf,g⟩ = ⟨f,Sg⟩`
decorre só da simetria; o `sin`/`cos` era decoração. Com entradas inteiras é exacta.

**(C) HONESTO.** A quantidade **é** transcendente. Aí o `double` é a representação certa,
mas **o que se AFIRMA muda**: em vez de «π = 3,14159 ± 1e-9», afirma-se o ENCAIXE
`223/71 < π < 22/7` — desigualdade exacta entre racionais, decidida em inteiros.

**How to apply:**
1. **Antes de escrever um limiar, dizer a que classe pertence.** Sem transcendente na
   conta, ele é decoração e não entra.
2. **O teste é olhar à volta**: `tools/triagem_limiares.sh` classifica 918 sítios em
   segundos, e separa (A) do resto com segurança.
3. **E a migração de tipos não é mecânica**: `tools/audita_tipos.sh` dá a matriz
   estrutura/realização/medição/apresentação. A ordem é **medição primeiro** (é defeito),
   estrutura depois (é alavanca), realização a seguir; apresentação fica.

`tests/limiares.c` (5:0), com os três casos tomados de código real.
Ver [[feedback-o-double-que-so-transportava]], [[feedback-normalizar-nao-e-medir]],
[[feedback-assercoes-vazias]].
