---
name: feedback-revisores-externos
description: "Compensam MUITO. Formato que funciona, e o padrão dos 18 defeitos que apanharam em 03/08"
metadata:
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
---

## O formato que funciona, e é o único

**Afirmações concretas numeradas, com números, para ATACAR.** Nunca "revê o documento".
Cada item com veredicto pedido: `VERDADEIRA / FALSA / VERDADEIRA-MAS-MAIOR-QUE-A-MEDIÇÃO`.
E dizer "não edites nada".

**E é preciso REPICAR.** Em 03/08 os três ficaram *idle* sem entregar. Um `SendMessage` a
pedir o relatório — repetindo os 2-3 itens que mais interessam e avisando que os ficheiros
mudaram — trouxe os três relatórios, e foram os melhores do projeto.

## O que apanharam em 03/08 — 18 defeitos, e o padrão

**Todos os graves eram do mesmo tipo: a asserção ERA o defeito.**

| defeito | porque nenhum medidor meu o via |
|---|---|
| "todo real é o **ponto fixo** de uma operação de inteiros" — na 1.ª página de dois documentos | π é transcendente; pontos fixos de GL₂(ℤ) são só quadráticos. E o texto **desmentia-se 11 linhas abaixo** |
| a alegação de segredo no enredo | o revisor **quebrou a cifra**: 2 pares → chave exata. É Hill (1929) |
| `prop:bijecao` "a menos de rotação cíclica" | a ressalva estava **ao contrário** — (1,2)→1,366 e (2,1)→2,732, uma classe e dois reais |
| `§P2/§P3` a sustentar `prop:degrau` | **circular**: medem a distância a um `x` que já existe. O ingrediente com conteúdo — `q_k → ∞` — não estava medido |
| `§P4`, `§P9`, `§C5` | `adj(M)·M = det·I` é identidade. **Não podiam falhar** |
| `§P11` a passar | corria em **overflow**; certo por sorte, via comportamento indefinido |

## As três regras que saíram

1. **Se a frase parece grande, procurar o contra-exemplo DENTRO do próprio parágrafo.** Duas
   vezes em 03/08 ele estava lá: o `t₀=2` que eu chamava centro falha o "inteiro mais
   próximo"; a "órbita" desmentia o "ponto fixo" onze linhas abaixo.
2. **O nome errado mata o resultado.** Chamar "polo" a uma ramificação logarítmica destruía a
   minha própria conclusão da fase π — em volta de um polo a monodromia é trivial.
3. **Reivindicar a mais custa mais do que dizer menos.** "A CONTRIBUIÇÃO" sobre material de
   Khinchin — com Khinchin na própria bibliografia — e a secção de Atribuições a dizer o
   contrário 496 linhas abaixo. Ver [[feedback-o-sujeito-da-frase]]: a regra é pôr o resultado
   como sujeito, não inflacionar o que ele é.

**Não delegar a DECISÃO do que aplicar** — verificar cada contraexemplo antes. Em 03/08
verifiquei os cinco principais e todos bateram; mas as correções *deles* que apliquei sem medir
teriam falhado duas vezes (o `k` a cortar, a monotonia em `k=1`).

Ligada a [[feedback-assercoes-vazias]] e a [[feedback-dois-caminhos]] — um revisor externo é
literalmente o segundo caminho.
