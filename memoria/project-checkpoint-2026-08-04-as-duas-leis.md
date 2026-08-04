---
name: project-checkpoint-2026-08-04-as-duas-leis
description: "Checkpoint 04/08 madrugada: a dualidade promovida a LEI em duas — a unidade é dual, a dualidade é dual — e tudo o resto (Newton incluído) como corolário"
metadata:
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-04T07:14:15.655Z
---

Madrugada de 04/08, 30 commits. A sessão foi **uma única afinação, feita pelo Aarão em cerca de
quinze correções seguidas**, e cada uma tornou o enunciado mais curto. Vale a pena guardar a
trajetória, porque o método é o resultado.

## ONDE PAROU: DUAS LEIS

> **Lei 1 — a UNIDADE é dual.**  `1 ~ −1`  ⟺ traço 0, `d = −a`  → a Möbius **involutiva**
> **Lei 2 — a DUALIDADE é dual.** `f⁻¹ = −f` ⟺ `σ' = −σ⁻¹` ⟺ `det = −1` → a de **Fibonacci**

São **autorreferentes em escada**: a primeira diz que o objeto mais simples que existe já é um par;
a segunda diz que a própria operação também tem dual — e essa é a bidualidade. E são os **dois
objetos que o texto já usava**: a involutiva que MEDE (o espelho, período 2) e a de Fibonacci que
ANDA (o gerador). O produto delas é o passo, `M = J·i`.

**E o `~` é TAMANHO, não igualdade** — foi o que eu percebi mal duas vezes. `1 ~ −1` diz que os dois
têm o mesmo tamanho e sinais opostos.

## DOIS ANDARES, que eu tinha confundidos

- **A existência** (três proposições, sem hipótese sobre o objeto): a escrita tem dual, a leitura
  tem dual, as duas são adjuntas. Prova: a transposição inverte a ordem e a inversão inverte-a
  outra vez — as duas trocas cancelam-se.
- **As duas leis**: dizem *que forma* o dual tem. Prova de uma linha cada.

## E TUDO O RESTO É COROLÁRIO — Newton incluído

| corolário | de onde vem |
|---|---|
| Newton 1, inércia | o tamanho não muda sozinho (Lei 1) |
| Newton 2, `∫F dt = Δp` | integrar a Lei 2 dá o que se conserva |
| Newton 3, `F_AB = −F_BA` | o sinal oposto da Lei 1 — e em Möbius é **literal**: `d = −a` |
| Newton 4, gravitação `1/r²` | a Lei 2 na forma nula, `∇²φ = 0` |
| `f=1`, `f=−1`, `f=f⁻¹`, `f=−f` | os quatro casos de sinal das duas |

**A gravitação deriva-se**, e o expoente sai da DIMENSÃO: `r^{d−1}·φ' = const` ⟹ `F ∝ 1/r^{d−1}`,
e em d=3 isso É `1/r²`. O expoente `d−1` é a **codimensão da esfera** — o mesmo `thm:codim` que
resolveu Hilbert. *A órbita que não dissipa e o inverso do quadrado são a mesma afirmação em
variáveis diferentes: a norma não se move no tempo, o fluxo não se move no raio.*

## O RESGATE: o Pégaso já era a Lei 2

Estava escrito nos três documentos e nunca ligado — `enredo:786` *"Pégaso, que voa em órbita e nunca
cai, porque é um rotor puro — a altura conserva-se"*; `catalogo:12974` *"o rotor (e^{iθ}, |·|=1)"*.

## A CURVATURA de cada corpo, e também já lá estava

É `Δ = tr² − 4det` (a classificação de Möbius): **J** hiperbólico (Δ=+4), **i** elíptico (Δ=−4),
o *shift* parabólico (Δ=0). E `f'' = K·f` com K a curvatura dá as três geometrias — *"curvatura
constante" e "não dissipa" são a mesma exigência*.

## AS CORREÇÕES DELE, por ordem — e todas tornaram o enunciado mais curto

1. *"são DUAS, f=f⁻¹ e f'=f⁻¹"* — eu exigia que UMA função cumprisse as duas.
2. *"a contribuição é organizacional?"* — fui verificar e **as equações já estão publicadas**
   (Cook; Wonenburger/Đoković 1967). Apresentei a fatorização como achado e é de 1967.
3. *"a segunda lei está errada: o passo é SEMPRE dual, independente da exigência"* — eu escrevera
   uma lei a pedir licença a uma coisa que já a tinha. **Contradizia a minha própria Lei 1.**
4. *"não é CONDIÇÃO para fechar, é O QUE FECHA — sempre fecha"*.
5. *"só f = −f, pois f = f⁻¹ é a 3"* — os eixos: inversa vs sinal.
6. *"não é f particular, `~` é tamanho"*.
7. *"duas leis apenas, Newton como corolário"*.

## OS MEUS DEFEITOS, e o padrão

**Medir por pipe.** `grep -c` num pipe grande devolveu vazio DUAS vezes e eu li o vazio como zero —
anunciei "margens a zero" com 5 e depois 3 caixas por corrigir. Agora **escrevo o log em ficheiro e
leio de lá**.

**Três caixas fora da margem introduzidas por mim** (128,7 · 61,5 · 41,4 pt): equações longas numa
linha só. Só se veem com `\hfuzz=0pt` — o `\sloppy` cala-as.

**Ferramentas apanharam o que a leitura não apanhou**: o `gcc` viu `-rb == -rb` (tautologia); a
bateria viu um medidor que deixou de compilar (281/282); uma asserção minha falhou e tinha razão
(`m=0` dá Δ=4, quadrado perfeito — é o nível 0, e as raízes ±1 SÃO racionais).

**E o bug do acento, reincidente**: `[a-zç]+` parou no "ç" e produziu `realizaç da Lei~2 (a
leitura)ão` em 12 etiquetas.

## ESTADO

`tests/escada.c` (22) e `tests/bidual.c` (28) novos. **Bateria 282/282.**
teoria 117 · catálogo 319 · enredo 299 · livro 738 pp. Catálogo e enredo com margens a **zero**.

Ligado a [[project-a-lei-em-dois-niveis]] e [[project-dualidade-memoria-da-divisao]].
