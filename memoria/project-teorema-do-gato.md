---
name: project-teorema-do-gato
description: "O Teorema do Gato — det É a medida; sobe em espiral, desce discreto; e o dual do gato é o PASSARINHO."
metadata: 
  node_type: memory
  type: project
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-16T00:11:33.972Z
---

**O Gato sobe em espiral e desce discretamente.** A lei: `det = σσ'` é o factor de escala
da medida, e daí `σσ' = −1` **DÁ** `|det| = 1` — a conservação da área.

**A seta vai só num sentido, e eu tinha escrito `⟺`.** O recíproco é FALSO: `|det| = 1`
admite `σσ' = +1`, que é a identidade e não é gato nenhum. A identidade é `det = σσ'`; o
`−1` é a ESCOLHA da torre, e é ela que traz a inversão de orientação junto com a
conservação. O controlo está medido ao lado (§M3).

**O conteúdo NÃO é `det|·| = σ_n`** — com `σ_n(A) := |det A|` isso é verdade por
definição e não pode falhar. São QUATRO contas sem código em comum a dar um número:
eliminação (álgebra) · cunha das colunas (geometria) · `σσ'` tirado dos COEFICIENTES sem
avaliar raiz (espectro) · contagem de pontos do reticulado, que não toca em determinante
nenhum (Lebesgue no discreto).

**As três vidas:** dualidade (Universal, a lei — e não a calcula) → volume (Peano,
realização algébrica) → medida (Estelar, realização analítica, e o Jacobiano mostra que a
lei é LOCAL: `T(x,y) = (x+y²+3y+5, y)` não é linear e `|det DT| = 1` em todo ponto).

**A BIDUALIDADE, e é ela que explica o nome.** O dual métrico inverte a medida:
`det(T*) = 1/det(T)`. Logo a 1.ª dualidade é `d ↦ 1/d` e NÃO é a identidade; a 2.ª é (o
bidual, Lei 1). E o gato é o **ponto fixo da primeira**: `d = 1/d ⟺ d² = 1 ⟺ |det| = 1`.
**Conservar a medida é ser o seu próprio dual métrico.** O dual do gato é o PASSARINHO —
o corpo na sua realização particular —, e o que os distingue é *quantas dualidades cada
um precisa para voltar a si*: o gato volta à primeira, a instância só à segunda. O Peano
é passarinho pela sua própria letra («este corpo é instância»), e o viveiro já dizia o
resto: fundir N corpos dá UM, o menor que os contém, «e voa».

**E o Universal é um corpo SOBRE outro corpo** — os seus elementos são as instâncias, e a
lei que guarda é uma lei sobre leis. Mesma forma da meta-indução um andar acima.

**O senão que corrige a fórmula fácil.** `D∘U` aponta, na notação do cone/espiral, para o
lado que NÃO fecha: a casa já tinha medido `Σ∘Π = Id` mas `Π∘Σ ≠ Id` — é RETRACÇÃO, não
involução, e a falha conta-se (todo racional tem exactamente duas expansões). `|det| = 1`
vale nos DOIS compostos e só um é a identidade: **a conservação da medida é estritamente
mais fraca que o fecho do laço.** O gato desce ao mesmo tronco e não às mesmas marcas de
unha.

**A PATA QUE SOME NA MARCA** (corolário). A projecção conserva a MEDIDA e perde a FIBRA.
Fecha como corolário e não como intuição porque corresponde a três construções já
formalizadas: a **dobra temporal** (`x ↦ x²`, «dois meios-passos são um tick», e ±√σ
colapsam — fibra 2, e o zero é o único com fibra 1), o **cone** (duas expansões, um real)
e a **sombra** do simplex. Controlo: num mapa injectivo a fibra é 1 em 401/401 — logo
«fibra > 1» não é automático.

**O LIVRO-RAZÃO**, e cada linha tem número: sobe conserva a medida e não esquece nada ·
espirala conserva sem tecto · desce conserva o real e esquece qual expansão · o cone
conserva a medida nos dois ramos e esquece a fibra (=2) · pisa a marca e esquece a pata ·
observa o traço e esquece o estado. Lidas juntas: **a medida atravessa a cadeia inteira;
o percurso não.**

**64 BITS SÃO DOIS DUAIS DE 32** (`lib/dual32.h`, `tests/dual32.c` 6:0). Multiplicar dois
de 32 dá um de 64: a operação SAI do tipo e guardar só 32 perde exactamente metade — «a
dualidade é a memória da divisão», literal. O par (alto, baixo) guarda-a, sem tipo largo
nenhum: as parcelas de 16×16 cabem num 32 porque (2¹⁶−1)² < 2³². E o **alto é a parte que
ORDENA**: comparar só pelo baixo erra em 12158 de 40000. Substituiu o `__int128` em
`an_cmp_quad` e `mt_pell`. Defeito meu que o medidor apanhou: emparelhei o determinante
como `a·b − c·d` em vez de `a·d − b·c` — 6560 divergências em 6561, e uma varredura só
com matrizes simétricas não o teria visto.

`tests/conservacao_metrica.c` (14:0) · `thm:gato`, `thm:meta-inducao`, `cor:bidualidade`,
`obs:passarinho` no Universal · `thm:det-volume`, `cor:passarinho-peano` no Peano ·
`thm:medida-jacobiano` no Estelar. Ver [[feedback-saturacao-nao-e-resultado]],
[[project-a-lei-em-dois-niveis]], [[feedback-justificar-o-que-so-e-coerente]].
