---
name: feedback-decidir-onde-nao-se-sabe
description: "Pus a recusa no sítio onde o dado que a justifica ainda não existe: o leitor da lista corre ANTES do GROUP BY, e a recusa derrubou dois medidores verdes."
metadata:
  node_type: memory
  type: feedback
---

`count(*)` com `sum(a)` é conflito real: o count corre pela soma do campo
(popcount) e as agregações pela varredura das células — dois percursos, duas
réguas para a mesma contagem. Escrevi a recusa **na leitura da lista de
colunas**.

**Só que com `GROUP BY` não há conflito nenhum**: ali o count de cada fibra *é*
o `G` que a corrida já conta, no mesmo percurso — e é assim desde que §W43 o
mede. A leitura da lista corre **antes** de se saber se há `GROUP BY`. A recusa
não podia distinguir os dois casos porque, no sítio onde eu a pus, a informação
que os separa **ainda não tinha sido lida**.

Resultado: `§W43` e `§W44` passaram a vermelho. Dois medidores verdes há muito,
derrubados por uma correção certa posta no sítio errado.

**Why:** eu escolho o sítio da recusa por *proximidade sintática* — «é aqui que
leio o `count`, é aqui que recuso» —, não por onde a **condição** está
disponível. É o mesmo erro do filtro que corria antes da reconstrução
([[feedback-o-filtro-antes-da-reconstrucao]]), noutra direção: lá agia cedo
demais sobre um campo por construir, aqui decidia cedo demais sobre um dado
por ler.

**How to apply:**
1. Antes de escrever uma recusa, perguntar: **de que depende ela, e isso já foi
   lido neste ponto?** Se não, marca-se (`agr_conflito = 1`) e decide-se onde a
   informação existe.
2. Uma recusa nova exige o controlo do **caso legítimo vizinho**. Eu tinha o
   controlo do «cada uma sozinha ainda responde» e não o do «a combinação
   legítima ainda passa» — e foi exatamente esse que faltou.
3. Correr o medidor inteiro depois de acrescentar uma recusa, não só o bloco
   novo. As duas falhas estavam em blocos que eu não tinha tocado.

Da família de [[feedback-a-recusa-que-deixa-rasto]]: as duas vezes o erro não
foi *recusar*, foi **onde** e **com que consequências**.
