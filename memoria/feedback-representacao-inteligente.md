---
name: feedback-representacao-inteligente
description: "A analiticidade é do OBJETO, não da representação. A representação certa faz o trabalho desaparecer — não o acelera"
metadata:
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
---

O Aarão, 03/08:

> *"a série que estamos estudando **já é analítica**, não importa como seja representada. Um
> computador computa tanto quanto um pau e uma pedra. Então **use a matemática a seu favor e
> represente de forma inteligente**."*

## O que isto significa, medido

O mesmo objeto — a solução de `x^x = n` — em duas representações:

| | custo | alcance em `n = 2` |
|---|---|---|
| inverter a série até grau 14 (Lagrange, racionais) | **98,8 ms** | `−34997` — **lixo** |
| Halley sobre `x = e^{W(ln n)}`, 8 iterações | **0,039 ms** | `1,559610469462` — exato |

**2542× mais rápido — e a série NÃO ALCANÇA ali**, por mais termos que se somem: `n=2` está fora
do raio `1−e^{−1/e} = 0,3078`.

> **Não é otimização. É a diferença entre chegar e não chegar.**

## A REGRA

**Antes de computar, escolher a representação em que a pergunta é trivial.** O objeto não muda; o
trabalho sim — e às vezes desaparece.

| em vez de | usar |
|---|---|
| somar a série | a **forma fechada** |
| varrer N pontos | a **recorrência** (desdobrar) |
| `double` com tolerância | **inteiros** com igualdade |
| dividir | **produto cruzado** |
| construir a base | a base que o objeto **já tem** |
| medir a discrepância com 2000 pontos | `σ_m` é crescente — **uma linha** |

## Por que eu erro isto

Vou para a **ferramenta genérica** (somar, varrer, aproximar) em vez de olhar para o objeto. E o
objeto do projeto tem sempre estrutura exata: `ℤ[σ]` é um par de inteiros, a recorrência é de dois
termos, `det = ±1` dá a inversa sem divisão, os traços são inteiros.

**O sintoma:** se o medidor demora, ou precisa de tolerância, ou de muitos termos — a
representação está errada, não a máquina lenta.

Ligada a [[feedback-inteiro-primeiro]] (o mesmo erro na aritmética) e a
[[feedback-a-base-ja-existe]] (o mesmo erro na álgebra). São três caras de um só: **não uso a
estrutura que o objeto tem e que o texto que escrevo descreve.**
