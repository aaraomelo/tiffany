---
name: feedback-o-endereco-ja-tinha-dono
description: "Pus zonas novas em ZONA(16..19) e escrevi por cima das ÁRVORES DOS ÍNDICES; e implementei comandos que o motor já tinha, passando à frente dos reais."
metadata:
  type: feedback
---

Duas formas do mesmo erro, no mesmo dia, e nenhuma delas deu colisão nenhuma na hora.

## A zona

Precisei de sítio para o plano alto, o corpo largo, o RLS e os DEFAULT. Escolhi
`ZONA(16)`, `ZONA(17)`, `ZONA(18)`, `ZONA(19)` — os números seguintes aos que eu via
declarados por perto. Lá viviam as árvores dos índices:

```c
#define S_IDXBASE(k)   (ISA_TECTO + ZONA(16 + (k)))   /* k até IDX_MAXCOL */
```

Uma macro com **parâmetro** não aparece quando se procura pelo número. O UNIQUE deixou de
funcionar, e o medidor que caiu estava a nove mil linhas de distância do que eu tinha
escrito. As zonas passaram a 24..27 e ficou um comentário no sítio a dizer porquê.

## O comando

Escrevi `CREATE INDEX` e `ALTER TABLE ADD COLUMN` porque o schema do cliente os usava.
O motor já os tinha — e o meu, declarado antes, **interceptava**: o real constrói a árvore
e faz o levantamento das linhas, o meu registava e seguia. Nove unidades caíram.

**Why:** o nome, o número de zona e a palavra-chave do comando são todos ENDEREÇOS, e um
endereço já ocupado não protesta: aceita a segunda escrita e responde a quem chegar
primeiro. O defeito aparece longe, noutro medidor, e parece de outra coisa.

**How to apply:**

1. **Antes de escolher um endereço, listar quem já lá está** — e procurar pela MACRO, não
   pelo número: `command grep 'ZONA(' banco/sql.c` mostra as com parâmetro, que a busca
   pelo literal não mostra.
2. **Antes de implementar um comando, perguntar se o motor já o faz.** Se faz e é preciso
   acrescentar-lhe algo, DELEGAR (`sql_executa_1`) em vez de duplicar. Duas
   implementações do mesmo comando é como elas deixam de concordar — a mesma razão pela
   qual o `serie.h` foi extraído em vez de copiado.
3. **E os tectos que a extensão deixou para trás.** Ampliar não é só arranjar sítio: o
   corpo passou a S_CORPOX_N = 32 e ficaram DEZ sítios com `j < 8 ? … : CORPO_INTEIRO`,
   mais o `S_COLNOME_N` em 8 — a tabela ficava com onze colunas e três sem NOME, que é o
   mesmo que não as ter. Ver [[feedback-a-cobertura-que-nao-acompanhou]].

Medido em `banco/sql.c`, 23/08. Liga a [[feedback-decidir-onde-nao-se-sabe]] (escolhi o
sítio por proximidade sintática) e a [[feedback-procurar-na-bateria-antes]].
