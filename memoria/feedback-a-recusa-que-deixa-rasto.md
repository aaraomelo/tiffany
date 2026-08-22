---
name: feedback-a-recusa-que-deixa-rasto
description: "O `CREATE TABLE` recusado deixava o ficheiro criado, e a tabela recusada passava a comportar-se PIOR do que uma que nunca existiu: aceitava SELECT e INSERT."
metadata:
  node_type: memory
  type: feedback
---

O analisador da lista de colunas parava na primeira palavra que não reconhecia
e **criava a tabela com o que já tinha lido**. `CREATE TABLE b (p INTEIRO COM
SINAL, q RACIONAL)` — sintaxe inventada no meio — dava uma tabela de **uma**
coluna, anunciada como criada. As consultas seguintes liam uma matriz 2×1 onde
estava escrito 2×2.

**E depois de eu pôr a recusa, o defeito mudou de sítio em vez de sair.** O
ficheiro da tabela abre-se ANTES de a declaração acabar de ser lida — tem de
ser, é nele que o catálogo vai. A recusa deixava-o lá, vazio e sem nome, e aí:

| | `SELECT` | `INSERT` |
|---|---|---|
| tabela nunca mencionada | «não existe» | «não existe» |
| tabela **recusada** | `SELECT 0` | `INSERT 0 1` |

A recusa era uma **criação encoberta** — e a recusada portava-se *pior* do que
a que nunca existiu.

**Why:** eu verifico que a recusa DEVOLVE erro e paro aí. A recusa não é um
valor de retorno: é um compromisso de **voltar ao estado anterior**. Um caminho
que já escreveu antes de recusar tem de desfazer, como a transacção faz.

**How to apply:**
1. Quando acrescentar uma recusa, perguntar **o que já foi escrito** até ali —
   ficheiros abertos, slots gravados, nomes registados.
2. O gume não é o texto do erro: é a **indistinguibilidade**. Comparar o objecto
   recusado com um que nunca existiu, e exigir a MESMA resposta. Comparar com a
   cadeia do erro passaria com o ficheiro lá.
3. Desfazer pelo caminho que já existe (aqui: o do `DROP`), não escrever um
   segundo.

Da família de [[feedback-o-medidor-que-nunca-mediu]] — verde por não poder
falhar — e de [[feedback-destruir-antes-do-inventario]]. O sintoma de entrada é
o de [[feedback-aceitar-e-fazer-outra-coisa]] se existir: aceitar em silêncio
uma declaração que não é a pedida.
