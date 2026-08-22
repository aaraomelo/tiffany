---
name: feedback-o-filtro-antes-da-reconstrucao
description: "Pus o filtro do dual antes de TRÊS caminhos que reescrevem o campo do zero (índice, subconsulta, join). A resposta passou a depender de haver índice — e o filtro era monotónico, logo movê-lo era de graça."
metadata:
  node_type: memory
  type: feedback
---

O filtro da presença (`WHERE c = 0` não casa com célula ausente) foi escrito
logo a seguir ao molde. Mas há **três** caminhos que reconstroem `S_MATCH` do
zero depois disso: a descida pelo índice, a subconsulta e o join. Os três
apagavam o filtro.

O sintoma foi o pior possível: **a mesma pergunta com duas respostas** —
`c = 0` devolvia $0$ sem índice e $1$ com índice. A régua da casa é que o
índice muda o **custo**, nunca a resposta, e eu tinha-a quebrado.

E havia uma segunda camada: a guarda estava condicionada a `tem_where > 0`, e
no caminho do índice **o molde não corre** — não há WHERE compilado. Ficava de
fora exactamente onde era mais precisa.

**Porquê:** escrevi o filtro onde o dado estava fresco na cabeça (a seguir ao
molde), não onde o objecto está **acabado**. Um filtro monotónico — que só
apaga — pode correr no fim sem custo nenhum, e no fim é o único sítio que não
depende de quantos caminhos existem hoje.

**Como aplicar:** antes de colocar um filtro, perguntar *quem escreve este
objecto depois de mim?* — `grep` pelo destino (`mem_grava(S_MATCH`), não pela
lógica. E se o filtro só apaga, pô-lo depois de tudo, por omissão.

A causa de raiz estava mais fundo e é a lição melhor: o índice **indexava a
célula ausente**, pondo o neutro na árvore com cara de valor. A árvore indexa
o corpo; o dual vive no bitmap. Corrigir a origem torna o filtro uma rede, não
a defesa.

Da família de [[feedback-dois-caminhos]] — foi a COMPARAÇÃO entre com-índice e
sem-índice que o apanhou, nenhuma asserção isolada — e de
[[feedback-duas-reguas]]: o sítio esquecido foi outra vez um caminho escrito
quando só havia um.
