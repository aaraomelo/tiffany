---
name: project-checkpoint-2026-08-23-o-envelope
description: "23/08 — a célula sobe ao plano alto (F_w ⊕ σF_w), o schema do ERP entra, o RLS é a erosão do mórfico; e alargar o limite derrubou dois medidores verdes."
metadata:
  type: project
---

# 23/08 — O ENVELOPE SOBE, E O ALCANCE DEIXA DE SER EXERCIDO

Commits `c1665043` e `599fba21`. `tests/pgwire.c` de 188 para **189 unidades**, todas
verdes; `sql` 98/98, `refs` 12/12.

## A célula ganhou o andar de cima, e é o thm:espaco

A célula era um byte assinado: −128..127. Passou a −32768..32767, com o byte baixo onde
sempre esteve e o alto num plano paralelo, `S_ALTO` — que é o `F_{2w} = F_w ⊕ σF_w`: a
segunda cópia MULTIPLICADA por σ, não uma segunda tabela. **Quem lê em baixo continua a
ler o mesmo**, e por isso toda a aritmética já emitida não mudou uma instrução. A DATA
usa três planos (`baixo | alto<<8 | alto2<<16,24`), que é o andar seguinte.

Com isso o `plataforma/tiffany-erp` entra no motor: **395 de 431 comandos** do schema
Prisma. Dos 36 que ficam, 34 são blocos `DO $rls$ … END` — PL/pgSQL, não SQL — e 2 são
índices sobre a décima primeira coluna, recusados porque as árvores de índice são oito
zonas. A recusa diz qual, que é o que ela tem de fazer.

## O RLS é a erosão do corpo mórfico

Não foi preciso mecanismo novo: *erode-se para ESCOLHER, dilata-se para escrever*. A
política aplica-se depois do WHERE, lendo `app.tenant_id` do catálogo. `acme` vê 2 linhas,
`globex` 1, o bypass 3 — e o `sum` erode com elas: 750 e não 1650. **Um agregado que
somasse tudo seria a fuga que o RLS existe para não haver.**

E o pool de texto passou a DEDUPLICAR, o que não é economia: a mesma cadeia tem de dar o
mesmo endereço, `x = y ⟹ R(x) = R(y)`. Sem isso `'acme'` na linha e `'acme'` no `SET`
eram dois objectos, e a política não casava com nada.

## O que a mudança quebrou — e o que isso ensina

**Dois medidores verdes caíram sem defeito nenhum.** Ambos exigem ver o envelope TRAVAR
alguma coisa (`if(fora_env == 0) mal++`), e com o envelope maior nada travava: com cinco
letras o maior convergente é 185, que cabia mal em 127 e cabe folgado em 32767. A lei não
mudou — mudou onde ela deixa de caber. A cura foi reencontrar o regime, não relaxar a
asserção: o m das famílias sobe a 1..4 (185/185 instâncias em 20/20), e a palavra passa a
ONZE letras com a base de cinco a repetir-se — **que é a palavra periódica, o quadrático**.
Ver [[feedback-o-gume-que-a-melhoria-desarma]].

**As zonas novas colidiram com as árvores dos índices** (`S_IDXBASE(k) = ZONA(16+k)`, uma
macro com parâmetro, que a busca pelo número não mostra), e os meus `CREATE INDEX` e
`ALTER TABLE ADD COLUMN` **interceptavam** os reais. Ver
[[feedback-o-endereco-ja-tinha-dono]].

## E os tectos que ficaram atrás do corpo

Tirar os interceptos fez o schema cair de 402 para 384 — e a causa não era o intercepto:
eram limites escritos quando as oito colunas eram tudo. O corpo foi a `S_CORPOX_N = 32` e
ficaram para trás o ALTER (recusava à nona, quando o CREATE aceitava onze — **dois tectos
para a mesma tabela**), o `S_COLNOME_N` em 8 (a tabela ficava com onze colunas e três sem
NOME, que é o mesmo que não as ter) e DEZ sítios com `j < 8 ? … : CORPO_INTEIRO`.

E a mensagem do `CREATE` **mentia**: a tabela de nomes tinha `INTEIRO` nos índices 5, 6, 7,
que são BOOLEANO, TEXTO e DATA. O corpo estava certo e o veredicto é que dizia outra coisa
— o pior sítio para um erro estar, porque nada falha.

`§W186` mede as três, e o gume custou uma correcção: com a data na PRIMEIRA célula a
asserção **passava sem poder falhar**, porque em (0,0) o endereço `i·ncols+j` vale zero
antes e depois. Posta na segunda coluna com duas linhas, a mutação que tira o `S_ALTO2` do
remapeamento manda a segunda data para `1970-01-01`.

## E o resto do dia

O `lib/serie.h` voltou ao que era — as séries formais do `calculo2.h` —, depois de eu ter
escrito por cima dele; a exponencial e o π em fatorial vivem em `lib/fatorial.h`. No
`catalogo.tex`, três `\ref` que atravessavam para o `aranha` passam a citar pelo NOME:
cada `.tex` compila sozinho, e lá sairia `??`.
