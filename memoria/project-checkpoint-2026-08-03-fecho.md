---
name: project-checkpoint-2026-08-03-fecho
description: "A arquitetura em lib/banco/tests, a bateria 279/279 pela primeira vez, e o padrão do dia: o cwd a duplicar tudo"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-04T00:50:47.314Z
---

Fecho de 03/08 — 101 commits no dia. A teoria está fechada; o trabalho foi **arrumar e medir**.

## A ARQUITETURA, que agora diz o que cada coisa é

```
lib/     20 cabeçalhos + tiffany.h      a biblioteca
banco/   13 .c + scripts + systemd      o que guarda e serve dado
tests/   265 medidores                   a validação
tools/   a bateria e as ferramentas
docs/    os 4 .md                        a raiz ficou só com .tex e .pdf
```

Nenhuma das 365 linhas `#include` mudou — a bateria passou a compilar com `-I../lib`.

## O PADRÃO DO DIA: o cwd a duplicar o banco

**Três vezes o mesmo defeito, e o maior valia 18 GB:**

| onde | o estrago |
|---|---|
| `regua.c` | 27 tabelas em TRÊS diretórios (raiz, tools/, tatoeba/) |
| `bairro/centro/dente/...` | 26 tabelas, 682 MB |
| `fita.c` | **`tools/.torre/` com 18 GB** — cópia inteira da fita e da povoada |

Todos escreviam no **diretório de trabalho**, e a bateria corre de `tools/`. O `fita.c` até
avisava na linha ao lado: *"um `.torre` relativo já me pôs 935 MiB no sítio errado, e o programa
não deu por nada"* — estava escrito e ficara por resolver.

**A regra que ficou:** o dado pertence ao que o gerou (o corpus, o repositório), nunca ao sítio de
onde se lançou o programa. O `fita.c` sobe até ao `.git`; o `tabelas.h` ancora ao corpus.

## O TATOEBA SAIU, e a razão foi medida

Os 17 medidores dependiam de `pares.tsv`, `por.tsv`, `lexico.txt`, `corpus.txt` — **todos no
`.gitignore`**. Num clone limpo nenhum é reproduzível.

Descobri-o **apagando 682 MB por arrumação e vendo seis medidores verdes cair**. Os atestados
provaram com o mesmo hash de fonte: `ancora …47 0` → `ancora …47 2`. O apagar não criou o
problema — **expôs** um que estava mascarado por estado local. Ver [[feedback-o-disco-limpo]].

## A BATERIA FICOU 279/279, ZERO FALHAS

Primeira vez. O que faltava eram as **colheitas** do ollama, não o serviço. E ao colher, o
`antissim.c` caiu — e a queda valia mais que o verde:

Ele exigia `per == 2`. A colheita nova deu **período 1** (o modelo repetiu sempre a mesma
resposta). A tese não caiu, ficou mais forte: o `hopfield.c` mediu que o lado que **mede** tem
período 2 e o que **ordena** tem 4; o que o medidor afirma é que ele caiu no primeiro. Período 1
ou 2 provam-no; **4 refutá-lo-ia**. Amarrar a asserção a um valor de um modelo não-determinístico
era medir a colheita e chamar-lhe teoria.

**E fica sabido:** a bateria não re-corre um medidor cujo *fonte* não mudou. Nos que dependem de
colheita, o verde pode descrever uma colheita que já não existe — o `--reatesta` é que resolve.

## AS MARGENS, e a caixa que anulava a correção

O `estilo.tex` tem `\sloppy`, que define `\hfuzz=.5\maxdimen` e **cala o aviso de Overfull**. Com
`\hfuzz=0pt` apareceram: teoria **123,94 pt** fora da margem numa caixa, mais 0,26 pt; catálogo e
enredo **zero**. Eram exatamente os dois que o Aarão viu.

E o mais instrutivo: passei a tabela a `longtable` e ela continuou apertada. A causa não era a
tabela — era a **caixa** que a continha. *Um `longtable` dentro de uma `minipage` não parte.* A
moldura rígida anulava a correção por dentro. `mdframed` resolveu.

## E NO FIM, UM RESULTADO NOVO: a família metálica sai de `f^{(n)} = f^{-1}`

O Aarão: *"só generaliza para a enésima derivada igual à inversa, que aí teremos a família
metálica completa. Demos a justificativa de que era a base a família metálica, está correto, mas
partindo da enésima derivada igual à inversa a transição fica evidente."*

A mudança é de **ordem de exposição**, não de resultado — e é ela que torna a transição óbvia.
Com $f(x)=a\,x^b$, derivar $n$ vezes baixa o expoente $n$ unidades e deixa à frente o produto
descendente $(b)_n$:

```
f^(n)(x) = a·(b)_n·x^(b−n)      f⁻¹(x) = a^(−1/b)·x^(1/b)
```

Igualar os expoentes dá `b(b−n) = 1`, isto é **`b² − nb − 1 = 0`** — que **é** a borda
`σ² = mσ + 1` com `m = n`. **O inteiro da derivada é o inteiro da borda.** Não há ponte a
construir entre a análise e a álgebra: há uma igualdade. n=1 ouro, n=2 prata, n=3 bronze.

O coeficiente, que a condição do expoente não fixa, fecha por `a^{1+1/b} = 1/(b)_n`. **Medido em
n=1..6 em VALOR e não só em expoente** (resíduo ~1e-16), e n=1 devolve `0,742742944625` — o número
já publicado para o ouro. A generalização contém o que lá estava, não o contradiz.

E a unicidade generaliza-se com ela: o discriminante `n²+4` **nunca é quadrado perfeito**, logo
σ_n é irracional para todo n e a família não tem um caso racional onde degenere.

**O medidor tinha uma tautologia**: comparava `cf[3]={1,-n,-1}` com `bd[3]={1,-n,-1}` — o mesmo
literal duas vezes. Agora o lado da análise **deriva-se** (expande-se `b(b−n)`) e o da álgebra
lê-se na borda. Ver [[feedback-a-referencia-escrita-a-mao]].

## O QUE FICA ABERTO

- ~80 buracos do `mutagera.py` por fechar (relatório com ficheiro:linha:coluna).
- `dualcifra.c` ainda usa DFT como régua emprestada — não corre sem ollama, não reescrevi às cegas.
- `.torre/` na raiz tem 79 GB (`grok/` 61 GB), fora do git.
- Os dados do tatoeba estão em `../tiffany-tatoeba/`, não apagados.

Deploy verde na Patria. Ver [[project-checkpoint-2026-08-03-noite2]] para a primeira metade do dia
(a DFT fora do `universal.c`, o `mutagera.py`, e o grep como substituto de medição).
