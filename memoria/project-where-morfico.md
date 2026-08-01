---
name: project-where-morfico
description: "O WHERE do sql.c é o corpo mórfico — erosão/dilatação, e elas são o par dual. Direção dada pelo Aarão em 31/07/2026, ainda por implementar."
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-07-31T14:23:30.258Z
---

O `WHERE` do `tools/sql.c` **é o corpo mórfico**: erosão e dilatação. Não é um filtro.

Eu estava a tratá-lo como filtro — um booleano que guarda ou deita fora — e a emendar a
identificação de coluna (o banco lê a coluna pela primeira letra, `sql.c:679` e `:1437`). Isso é
conserto de superfície. O que o Aarão apontou é a semântica:

- `WHERE` é **erosão** por um elemento estruturante: sobrevivem as linhas cuja vizinhança cabe no
  elemento. Estreita o conjunto.
- O **dual é a dilatação**: o que alarga. E é ela que fecha o `UPDATE ... WHERE` — **erode-se para
  escolher, dilata-se para escrever de volta**.
- O par fecha pela **adjunção**, que é a régua do mórfico (já registado: *"a máscara para o
  mórfico — a régua dele é a adjunção, e o raio ordena"*; no catálogo, `(B,C) = (2,1)`, parabólico,
  regime PA, *"dil por B_r — o RAIO soma"*).

**Why:** isto explica a recusa que apanhei — `erro: o WHERE não foi entendido — a consulta é
RECUSADA`. Um `WHERE` que não é elemento estruturante válido não é "não analisável": é um elemento
**sem dual**, e o banco a recusar é o banco a recusar meia peça. Bate com tudo o resto do sistema:
nada fecha sozinho.

**How to apply:** ao mexer no `WHERE`, não perguntar "que predicados aceito"; perguntar "qual é o
elemento estruturante, e qual é a dilatação que lhe corresponde". Um `WHERE` novo só está pronto
quando o seu dual também está. E o raio do elemento é o que ordena — é por aí que se compara um
`WHERE` com outro, não por contagem de linhas devolvidas.

Ver [[project-checkpoint-2026-07-31]] para o estado da migração da mina para o banco, que é onde
isto apareceu (o `fechar_faixa` continua por fechar à espera disto).

## E o PoW é via SQL — quem executa a task é o BANCO

Segunda direção do Aarão, no mesmo dia. Eu estava a construir um minerador em Python que *fala*
com o banco. Errado: **o banco é o minerador**.

O `sql.c` já compila para a ISA e corre no metal com a memória no disco — cada consulta imprime
`-- 50 bytes de ISA [709d], 0 átomo(s), 20 passos`. `SELECT` e `UPDATE` já são tarefas executadas
pela máquina do banco. O PoW é mais uma: o martelo (duplo SHA-256 sobre a faixa de nonces) é
**emitido para a ISA** e corrido pela mesma máquina.

O que isso arruma de uma vez:

- não há processo a cair, porque não há processo — o estado está no disco e a máquina retoma;
- a share é escrita pela mesma máquina que a encontrou, sem ida e volta;
- e casa com o mórfico: **a erosão escolhe a faixa a martelar, a dilatação escreve a share de
  volta**. O par mórfico é que faz o trabalho, não um laço de fora.

**How to apply:** não escrever minerador que chame o banco. Emitir o martelo no `sql.c` como se
emite qualquer outra operação, e deixar a máquina dele executar. O `mina_banco.py` que escrevi
(subprocess a chamar `sql`) é andaime, não a coisa — serviu para achar a gramática e deve cair.

## E a régua do PoW é a ELÍPTICA — não há corpo mineral à parte

Só há um corpo; todos são isomorfos. **A questão é escolher a régua, e a régua escolhida define o
corpo adequado na hora.** Para o PoW a régua é a **elíptica**, e o corpo métrico já o dizia
(`metrica.c §M5`): *a norma é definida positiva exatamente quando a assinatura é negativa* —
**nada tem norma zero fora do zero, logo não há cone nulo**.

É disso que o PoW vive. "Este hash está abaixo do alvo" só é pergunta honesta se quem passa formar
uma **bola** — fechada, limitada, à volta da origem. Na elíptica é: os níveis da norma são elipses,
o alvo é um nível, a share é um ponto lá dentro.

- **hiperbólica** tem cone nulo: haveria candidatos a passar o alvo **sem trabalho**, ao longo do
  cone. Não é feiúra — é a mina a poder ser roubada pela régua.
- **parabólica** é degenerada, o absorvente sem dual: colapsa o que lhe passa à frente.

**A dificuldade é o RAIO.** Alvo mais baixo é bola mais pequena. A dificuldade da rede deixa de ser
número mágico do protocolo e passa a ser a grandeza que já ordena o elíptico — o mesmo raio que
ordena o mórfico, e é por isso que o `WHERE` e o alvo se falam.

Resolve também os 256 bits: a comparação é **a norma**. A norma dos pares altos é uma cota (peneira
barata), a norma completa decide — mesma régua em raios diferentes, não duas coisas, e por isso a
peneira **nunca pode discordar** da decisão.

**How to apply:** `OP_MARTELO` compara por norma na régua elíptica. O SHA-256 continua em bytes
porque é a rede que o define (não é coordenada minha, e re-coordená-lo daria outro hash e nenhuma
share); tudo o resto — nonce, alvo, comparação, share — vive na cifra.
