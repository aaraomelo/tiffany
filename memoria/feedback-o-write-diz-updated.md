---
name: feedback-o-write-diz-updated
description: O Write diz «updated» quando o ficheiro existe e «created» quando não — e eu escrevi por cima de DOIS medidores no mesmo dia sem ler a palavra.
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-16T23:35:12.925Z
---

Duas vezes na mesma sessão criei um medidor «de raiz» e escrevi por cima de um que já
existia — e que media **outra coisa, melhor**:

- `tests/esquilo.c` media o esquilo como **acção à direita** (ℍ^op, a transposta que
  inverte a ordem, e `A(xB) = (Ax)B` sempre — a comutatividade que faltava). Eu escrevi
  por cima com os «dois ramos de |det| = 1». O que se perdia virou o teorema.
- `tests/natural.c` media **naturalidade** (`J : V → V*` não transporta, `ev : V → V**`
  transporta; o discriminante é λ², e λ² = 1 É a estaca), com **15 asserções**. Eu escrevi
  por cima com aritmética natural. Palavra igual, assunto diferente.

**Why:** o sinal estava lá as duas vezes e eu não o li. O Write responde
**«has been updated successfully»** quando o ficheiro existia e **«created»** quando não.
São palavras diferentes e eu tratei-as como a mesma. Nas duas vezes só o
`git status --short` ou o `git diff --stat` é que me mostrou o estrago, e por acaso.

**How to apply:**
1. **Antes do Write, `git ls-files | grep <caminho>`** — ou qualquer verificação de que o
   nome está livre. Nomes bons são escassos: `esquilo`, `natural`, `primitivas` são
   exactamente os que a casa já usou.
2. **Ler a palavra que o Write devolve.** «updated» num ficheiro que eu julgava novo é um
   alarme, não uma confirmação.
3. E quando acontecer: **repor do HEAD primeiro**, antes de qualquer outra coisa, e só
   depois decidir onde vive o conteúdo novo. Renomear o meu, nunca o que já lá estava.

Ver [[feedback-destruir-antes-do-inventario]], [[feedback-o-write-que-diz-updated]],
[[feedback-a-base-ja-existe]].

## 16/08/2026 — a TERCEIRA, e desta vez foi o TOTAL que a apanhou

`tests/potencia.c` media a potência como **terceira operação** — clone (o corpo e o
elemento ficam), reprodução (o corpo cresce, ℝ^lcm), potência (o ELEMENTO cresce, x⊗x) —
com a órbita a fechar em período de **Pisano**. 153 linhas. Escrevi por cima com o
cruzado invariante a potências. **Palavra igual, assunto diferente — outra vez.**

Li a palavra «updated» e não parei. E o `git ls-files` da regra 1 também não fiz.

**O que a apanhou foi a bateria não subir.** Acrescentei um medidor citado no paper e o
total ficou em **500**, quando tinha de ir a 501. Não foi uma asserção: nenhuma podia
ver isto — foi o TOTAL, que é a única coisa que conta medidores em vez de os correr.

**How to apply, o que se acrescenta:**
4. **O total da bateria é o detector deste defeito.** Sempre que um medidor novo entra
   citado num paper, o total sobe em um. Se não subiu, ou não entrou na lista ou
   substituiu alguém. Ver [[feedback-o-exit-sombreado]] — a mesma família: ler o número
   que conta, e não só os que passam.
