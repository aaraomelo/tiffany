---
name: feedback-destruir-antes-do-inventario
description: Substituí o teoria.tex enquanto o agente que eu próprio lancei ainda o estava a inventariar — e quase perdi 12 medidores em silêncio
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T01:25:04.582Z
---

02/08/2026. Lancei um agente para garimpar o `teoria.tex` (52 pp.) e dizer, peça a peça, o que ia
para onde. **E substituí o ficheiro enquanto ele lia.** O relatório abriu com o aviso: *"a
dissolução aconteceu enquanto eu lia"*.

## O que quase se perdeu

- **30 páginas** de resultados próprios com medida (Rogers–Ramanujan e a lei Q_m, o corte de
  Kronecker–Weber, `det B = Pf(B)²`, a densidade de ouro 5/(m²+4), Isserlis `N_k = k!!`, a
  dissociação do AGM, o selo, o prensor). Confirmei por grep: **zero ocorrências** nos dois
  documentos novos.
- **12 medidores**, que saíram da bateria **em silêncio**: as suas citações só existiam nas secções
  não recuperadas. Total de 276 → **264**, e **nenhuma falha apareceu** — um medidor que não corre
  não falha, desaparece. É o risco que [[feedback-revisores-externos]] já regista, e desta vez fui
  eu a causá-lo.

## O erro, e é de sequência

Pedi o inventário *porque* sabia que não podia apagar às cegas — e depois apaguei antes de o ter.
A pressa não foi por urgência: foi por o pedido seguinte do Aarão ter chegado e eu ter querido
avançar com ele.

**A regra:** quando lanço um agente para inventariar algo antes de o destruir, **o destino do
original fica congelado até o relatório chegar**. Não há "vou adiantando" — adiantar É destruir.

## O que salvou

1. O **git** (o antigo estava em `HEAD`, recuperável por `git show`).
2. O agente ter **aberto o relatório com o aviso** em vez de o entregar como se nada fosse.
3. Eu ter **comparado as listas de medidores antes e depois** em vez de olhar para o verde. O total
   caiu 12 e continuava a dizer "274 verdes, 2 falhas" — exatamente como antes.

## O teste que tem de ficar

Antes e depois de qualquer reorganização de documentos:

```
grep -ohE '(tools|tatoeba)/[a-z_0-9]+\.(c|py)' <os .tex que a bateria lê> | sort -u > lista
```

e `diff` das duas listas. **A contagem de medidores é o invariante**, não o número de verdes. Ver
[[feedback-dois-caminhos]]: o veredicto verde de uma bateria menor parece igual ao de uma bateria
inteira.

E os `.tex` que a bateria lê não são "papers soltos" — são **fontes da lista**. Estão em
`tools/bateria.sh` linha ~76.

**E a variante que quase passou:** reescrevi UMA secção de um medidor com
`s[:i] + novo` onde `novo` terminava em `s[j:]` — e entre `i` e `j` havia **outras três
secções**. Apaguei 242 linhas e **três** secções (§L11m, §L11n, §L11o) sem tocar em nada
delas. O alarme foi o TOTAL: **57 → 52**.

**How to apply:** um `s[:i] + novo + s[j:]` só é seguro se eu SOUBER o que está entre i e j.
Antes de recortar por índices, listar o que o intervalo contém — `grep -o "§L[0-9]*[a-z]*"` —
e depois do write conferir o TOTAL. E a recuperação é barata quando o commit anterior existe:
`git show HEAD:ficheiro` dá o bloco de volta letra por letra. Ver [[feedback-o-write-diz-updated]].
