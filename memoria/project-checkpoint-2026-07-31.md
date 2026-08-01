---
name: project-checkpoint-2026-07-31
description: "Checkpoint 31/07/2026 — a cifra virou o único sistema de coordenadas; texto, número e corpo na mesma tabela; hipercorpo e venom no catálogo; e o construtor que dispensa o trabalho de caso"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-07-31T07:56:02.309Z
---

# Checkpoint 31/07/2026 — tiffany

Estado: **159 medidores, 0 falhas**, `teoria.tex` com **59 páginas, 0 pendências**, `sql.c` com
**78 asserções**. Tudo empurrado no `master`.

## O que fechou hoje

**A cifra é o único sistema de coordenadas.** Texto entra símbolo a símbolo, racional por
Euclides, corpo pela deformação — todos na mesma tabela, todos comparáveis. `DISTANCIA TEXTO`,
`BUSCA TEXTO`, `ACHA TEXTO`, `INSERT TEXTO`, `CORPOS` no `sql.c`.

**O índice é a própria posição.** O lugar de uma entrada é a sua cifra: cada termo é um nível,
descer o caminho *é* medir a distância. Sem tabela de tamanho fixo, sem colisão, sem sondagem. O
custo é o comprimento da cifra, não o número de linhas.

**Os 30 corpos no catálogo**, com régua `(B,C)` lida do operador, cifra completa dos dois lados
(Wick), 18 lugares distintos, matriz de distâncias métrica. Entraram o **hipercorpo** (a reta do
rei deformada no tesseracto pela curva de Hilbert) e o **venom** (avançar e esvaziar são o mesmo
ato; `A_1` do lado próprio — o rei — e `A_16` do dual).

**O construtor** (`tools/constroi.c`, e depois generalizado dentro do `sql.c`): dada a cifra e a
deformação, o corpo sai. `B` = razão, `C` = sinal — *a régua é a deformação escrita em dois
números*. O contrato verifica-se **na** construção, não depois.

**O enredo confirmou.** `chess/sandbox/reino_dourado_enredo.tex` já tinha a teoria do venom escrita
antes: "a malha do girassol uma dimensão acima, no cone nulo, onde a distância própria é nula...
sobe e baixa, reversível" e "sessenta e quatro lances ... como os quatro de um mate do bobo" — o
`64/4 = 16` bate o `2^N` que eu tinha tirado da razão do nível da curva sem ter lido.

## Os meus buracos, e são sempre o mesmo

Ver [[feedback-nunca-usar-ram]] para a outra regra dura. Estes são de método, e o Aarão apanhou
cada um:

1. **Coordenada inventada.** Ia indexar a cifra escalando-a por um `N` arbitrário. *"só tem um
   sistema de coordenadas pra tudo sem inventar coordenadas arbitrárias."*
2. **Cortar a régua para caber no objeto.** Tectos meus por toda a parte: 128 termos, 111
   caracteres, `LARG 256` no termo. *"a régua é infinita, o objeto é que acaba"* — e o termo pode
   ser zero, negativo ou grande.
3. **Trazer o Δ de volta** quando a cifra "não fechava". Não era limite da cifra: era a cifra
   **incompleta**, com um lado só do chicote. *Se não fecha, falta-lhe o dual.*
4. **Varrer o fractal ponto a ponto.** *"vai demorar muito pra formalizar ou vai medir muito
   fractal ponto a ponto ainda?"* — num objeto auto-similar, varrer é medir mil vezes a mesma
   coisa. As contagens saem em duas linhas.
5. **Inventar dúvida onde a estrutura não deixa nenhuma.** Ia escrever que Hilbert era "o
   representante mínimo de uma família" e que a escolha era minha. Os 16 sub-cubos **são** os 16
   vértices do tesseracto; Gray, Hilbert e a recursão são a mesma coisa.
6. **Chamar defeito à outra metade.** Escrevi que o hipercorpo "não fecha do seu lado". *Nada fecha
   sozinho* — a seta de Wick diz **qual** das metades carrega o real, não "falha".
7. **Trabalho de caso a cada corpo novo.** *"toda vez eu te dei a cifra e a deformação infinita e vc
   sempre cai no mesmo buraco, não está claro que isso basta pra construir o corpo?"*

**Como aplicar:** antes de acrescentar mecanismo, perguntar se a cifra e a deformação já não o dão.
Antes de escrever um limite, perguntar se é do objeto ou meu. Antes de chamar defeito a alguma
coisa, procurar de que dual ela é a metade.

## Uma medida que deu NÃO

O venom **não** mede strings melhor: 58% contra 61% da régua do prefixo, contra Hamming — a
referência que eu escolhi *a favor* dele. Eu tinha previsto que ganhava. Fica no catálogo como
corpo; medir strings não é o ofício dele.

## Aberto

- A régua `(B,C)` de vários dos 28 vem de escolha minha nas famílias paramétricas (o gato `A_m`, a
  dilatação por λ): tomei o operador que o catálogo *nomeia*, e onde o parâmetro é livre, o membro
  mínimo ainda não tomado. Está anotado corpo a corpo no `CORPO28` para se poder contestar.
- `emit_atomos` emite *palavra* em vez de aritmética; decompor um unimodular arbitrário em palavra
  (Euclides); a comparação por norma *emitida* dentro do corpo quadrático.
