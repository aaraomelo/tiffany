---
name: project-o-real-e-o-corte
description: "ℝ na assistente — o real é o CORTE e nunca um decimal; e os três caminhos (corte, ponto fixo de Möbius, fração contínua) fecham um contra o outro."
metadata: 
  node_type: memory
  type: project
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T04:48:35.585Z
---

O andar de ℝ fechou a escada aritmética (commit `456fdd5`, `lib/reais.h`, §C29 com dez
unidades). A frase é dele: **«ℝ acrescenta a completude: todo buraco racional recebe um
ponto»**, e a cadeia completa está em [[project-escada-aritmetica-n-z-q]].

**Aqui a regra da casa deixou de ser disciplina e passou a ser a MATÉRIA.** Um `double`
afirmaria que o real é uma tira de casas; o corte diz que ele é a **decisão** sobre cada
racional. A decisão é inteira — `(p/d)² < a` é `p² < a·d²` — e por isso √2 mede-se sem
nunca se aproximar de nada. O ficheiro tem zero doubles: a única vez que a palavra
aparece é no comentário que os proíbe. Isto responde de vez ao [[feedback-inteiro-primeiro]]:
não é que o exato seja *possível* aqui — é que o aproximado seria **outro objeto**.

**TRÊS CAMINHOS, e a medida é a concordância deles:**

| caminho | papel | onde |
|---|---|---|
| o **CORTE** | decide | `rz_cmp`, inteiro |
| o **PONTO FIXO** | persegue | `x ↦ (a+bx)/(x+b)`, Möbius INTEIRO, ponto fixo `x² = a` |
| a **FRAÇÃO CONTÍNUA** | escreve | `lado` (cifra.h), periódica por Lagrange |

E fecham um contra o outro: **o convergente da FC cai dentro da caixa que a bisseção
fechou**, sem que os dois métodos se conheçam. É o [[feedback-dois-caminhos]] com três.

O `b` do Möbius **não se escreve à mão** — sai de `rz_b` com `b² > a`, e é essa condição
que faz a sucessão não trocar de lado (monótona, o §9 dele). Escrevi-o primeiro à mão
como `2` e a função ficou morta; ligá-la fez duas asserções passarem a morder na mutação.

**O que o andar acrescenta à teoria da casa**: o real é o **ponto fixo de um Möbius
inteiro**, isto é, o inversor da casa outra vez (`ν(x) = −1/x` e a órbita de Euclides — ver
[[project-checkpoint-2026-08-09]]). A sucessão de convergentes 1, 4/3, 7/5, 24/17, 41/29 é
a órbita, o corte é o ponto que ela persegue, e a FC é a palavra. **Os três já estavam na
casa separados; o andar de ℝ é o que os nomeia como o mesmo objeto.**

O **gume** da irracionalidade ficou no quadrado perfeito: a prova por fatoração (√a ∈ ℚ ⟺
todo expoente primo par — geral, e a paridade é só o caso a = 2) **PARA** quando a é
quadrado, e diz porquê. Uma cadeia que corresse na mesma provaria o falso.

Falas: `corte de raiz N`, `prova que raiz de N nao é racional`, `prova as provas` (o
índice das vinte), `prova N` e `prova <nome>`.

## Cauchy — a construção DUAL (`lib/cauchy.h`, commit `7081fec`)

O corte diz **onde** o ponto está (decisão sobre ℚ, estática); a sucessão **vai lá**
(caminho por ℚ, dinâmica). O mesmo real sai dos dois, e a equivalência mede-se: «os três
caminhos são o mesmo real» deixou de ser figura e passou a ser `aₙ − bₙ → 0`.

- **A sucessão é a REGRA, não a tabela** — nenhum termo se guarda, cada aₙ recalcula-se
  do princípio. É o [[feedback-nunca-usar-ram]] aplicado à análise.
- **O ε é um RACIONAL e o N exibe-se sempre**: 1/10 → N=1, 1/10³ → 4, 1/10⁵ → 7, 1/10⁷ → 9.
  Não há épsilon-delta com vírgula; há frações e um índice.
- **O GUME é a HARMÓNICA**: os saltos consecutivos são 1/(n+2) e vão a zero, e ela NÃO é de
  Cauchy (H₂ₙ − Hₙ ≥ 1/2). É o contra-caso que impede a definição de colapsar em «os termos
  consecutivos aproximam-se». Mutá-la para Basel (que É de Cauchy) derruba duas asserções.
- **Onde a completude se paga**: a MESMA sucessão, com os MESMOS termos, converge em ℝ e não
  em ℚ. A diferença entre os andares não é uma definição — é este par de medidas sobre o
  mesmo objeto.

**O defeito que quase passou**: escrevi `cy_equiv` a devolver o PRIMEIRO k com
|aₖ − bₖ| < ε, e as três sucessões do andar arrancam todas em ⌊√a⌋ — dava N = 0 por
COINCIDÊNCIA no arranque, e a asserção passava sem medir convergência nenhuma. «→ 0» é
sobre a **cauda**. Corrigido, os N passaram a 17, 6 e 17. Ver
[[feedback-assercoes-vazias]]: a forma nova é *a asserção que passa pelo arranque*.

**O que ele anunciou a seguir**: a construção geométrica da reta — «a reta não precisa ser
postulada como uma coleção contínua de pontos; o ponto real é o corte que separa tudo o
que ficou abaixo de tudo o que ficou acima». É o «raiz → corte → folha» que o motor já usa,
e liga ao commit `9a85359` (a reta real geométrica).
