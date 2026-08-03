---
name: project-checkpoint-2026-08-03
description: "03/08 madrugada: o romance ficou sem uma fórmula, o repo ficou auto-contido, e a involução ganhou justificação física — em quatro tentativas"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T06:29:11.223Z
---

03/08/2026, madrugada. **28 commits.** A sessão fez três coisas grandes e apanhou sete defeitos que
já estavam publicados.

## O estado, agora

| | | |
|---|---|---|
| `teoria.tex` | 72 pp. | álgebra · topologia · análise |
| `catalogo.tex` | 310 pp. | medidores, corpos, realizações, GDD |
| `enredo.tex` | 297 pp. | **zero fórmulas inline** (eram 199) |
| **`livro.tex`** | **683 pp.** | os três num volume, por `subfiles` |

**277 medidores.** Os quatro PDFs no ar em goldenkingdom.patriatechnology.com.

## 1. O romance ficou sem uma fórmula

199 inline → **0**; ficam 3 displays deliberados (σσ'=−1 no clímax, o `−1` sozinho, o câmbio da
Turmalina). Em quase todos os casos **a frase de substituição já estava escrita ao lado** — o autor
tinha-se traduzido a si próprio. Ver [[project-tres-documentos]].

Depois disso, ainda sobraram **nove palavras técnicas** (involução, impedância, isomorfo,
denominador, parâmetro, invariante) — a limpeza dos símbolos não apanha o léxico. O `dual` fica:
aparece 55 vezes mas **sempre com a explicação ao lado**, e um romance pode ensinar uma palavra.

## 2. O repo ficou auto-contido (o chess saiu)

Vieram o `app/` (25 ficheiros) e as `figuras/` (211, 117 MB — a única exceção à regra de não subir
binários: são rasterizadas na GPU). **Um workflow só**, em `texlive/texlive:latest`. O R2 saiu (os
secrets nunca existiram aqui e o `scp` sempre funcionou).

**O defeito que isto revelou:** o deploy do front dizia `success` **sem publicar** — um
`for i in 1 2 3` que não falhava o passo. Dois dias com o build de 01/08 e dois runs verdes por
cima. Ver [[feedback-assercoes-vazias]]: não um teste vazio, um **deploy** vazio.

## 3. A cadeia nova, que atravessa os três documentos

- **A Armadura Universal é a túnica/`headjack`** — e os dois lados já o tinham escrito sem se
  conhecerem. Ver [[project-armadura-e-tunica]].
- **O relógio de luz**: `t = N·d/(c·cos θ)`. *O tempo é o preço de se entrar torto*, e a Torre tem
  andares porque cada um tem um ângulo. O `cos θ` é o **mesmo** que o fator de potência e o lapso —
  mas **recíprocos** na lei: o tempo é o inverso do fator de potência (produto medido 1,0 em quatro
  ângulos).
- **A escada do diabo** fechou o argumento do romance: `1/2` é o único ponto fixo não trivial de
  `x↦−x`, logo **não tem dual a apresentar na fronteira** — a recusa de Benjamim é estrutural, não
  moral. Ver [[project-escada-do-diabo]].
- **A agulha de Dirac**: o deslocamento de uma casa não muda invariante nenhum e deixa um marcador
  inteiro. *Dark não abriu o andar de baixo: assinou-o.*
- **A involução justifica-se pela conservação**, com alcance limitado. Ver
  [[feedback-justificar-o-que-so-e-coerente]] — passou por **quatro** estados numa noite.

## 4. Sete defeitos que já estavam no ar

1. o `teoria.pdf` imprimia `Observação 15.1 (colback=ouroclaro!35,…)` — 8 vezes
2. `obs:ouro5` citada 4× com o label inexistente → **"Observação ??"** publicado
3. um `\appendix` a meio do catálogo, a marcar o GDD como apêndice
4. o design dourado do enredo só nos últimos **4%** do livro
5. o deploy do front verde sem publicar (dois dias)
6. dois "exemplos de involução" com **ordem 4**
7. duas frases falsas minhas sobre a escada (um revisor apanhou-as)

**Nasceu daí `tools/refs.c`** (276 → 277): `\ref` órfã, label duplicado, referência cruzada entre
documentos, e um **controlo negativo** que injeta os três defeitos e exige que o medidor os apanhe.

## O que fica por fazer

- **A revisão externa aos três documentos não aconteceu.** Doze agentes lançados, **um entregou** —
  e valeu por todos. O que funcionou: **uma pergunta única e específica**, enviada depois de o
  agente já estar idle. Briefings longos não voltaram nenhum.
- As 30 intervenções do narrador: 7 convertidas para primeira pessoa, o resto fica impessoal de
  propósito (onde "este livro" é contrato, não pessoa).

## A lição do dia, em duas linhas

**Os números estavam sempre certos. O que era falso era a frase que os ligava** — três vezes na
mesma noite. E: [[feedback-procurar-na-bateria-antes]] — o `colheita.c` já media `R+T+A=1` com a
absorção enquanto eu escrevia "três saídas e só três".
