---
name: project-escada-paga-uma-fibra
description: Cada andar torna UMA fibra total e paga noutra — e os `if` da aritmética são o preço de um andar, não defeitos.
metadata:
  type: project
---

**A LEI DA ESCADA.** Cada andar torna uma fibra total e paga noutra, e não há andar onde
tudo seja total:

| andar | fica total | paga-se |
|---|---|---|
| ℕ | a soma | a subtracção (`a+x=b` só se `b≥a`) |
| ℤ | a subtracção | o sinal |
| ℚ | a divisão | o zero (`0⁻¹`) |
| ℙ¹ | a inversão | a soma (`∞+∞`) |

**«Não eliminamos as excepções; descobrimos em que andar elas são o preço.»** Isso muda o
estatuto de tudo o que a casa escrevia como excepção: deixam de ser defeitos e passam a ser
uma quantidade, que se conta. O `0⁻¹` não sobrevivia à escada — era o preço de UM andar.

**OS `if` SÃO O PREÇO, E CADA UM TEM UM ABSORVEDOR COM NOME.** Os dez `if` do
`racionais.h` testam três coisas — sinal, zero, tecto — e as três vêm da NORMALIZAÇÃO.
Normaliza-se porque em ℚ os números crescem; em `𝔽ₚ` nada cresce, logo não se normaliza, e
sem normalizar: o sinal não existe (tipo), o zero é a troca `[q:p]` (representação e
dualidade), o tecto não tem para onde ir (tipo), o `[0:0]` fica no enunciado (domínio).
Contagem **na fonte**: 10 → 8 → 14 → **0** (`lib/sem_ramo.h`).

A disciplina, do eval: **não eliminar `if` por estética — só quando a condição é absorvida
por primitiva, tipo, domínio, dualidade ou representação; senão só se desloca a excepção.**
E a prova de que nada foi deslocado é comparar EXAUSTIVAMENTE com a versão ramificada:
16128 comparações, zero divergências. E o gume: há um `if` que NÃO se tira — passar do par
ao índice precisa de saber se `q = 0`, porque o índice do ∞ é convenção, não valor do
corpo. Por isso essa função existe para IMPRIMIR, não para calcular.

**A EXAUSTÃO, E O ENCAIXE.** 127 é primo E é o topo do `int8_t`, logo `𝔽₁₂₇` cabe inteiro
no tipo e `|ℙ¹(𝔽₁₂₇)| = 128` — os valores não negativos, com `∞ = [1:0]` a ocupar um lugar
como os outros. A regra do eval: **«não representar o infinito como um número grande —
representá-lo como o dual projectivo de zero».** Aí a inversão é bijectiva nos 128, o gato
é bijecção para todo metal, e a órbita FECHA sempre (períodos 6–128) — logo **não existe
prova por crescimento**, que era a classe de defeito do dia inteiro.

**E A ARITMÉTICA NATURAL.** Em ℕ um racional É uma sequência de naturais, e
`|pₙqₙ₊₁ − pₙ₊₁qₙ| = 1` (74651 pares) dá a forma fechada `1/(qₙqₙ₊₁)`. O módulo de Cauchy
deixa de ser busca na órbita: `qₙ·qₙ₊₁·a > b`, uma comparação de naturais. Formar a
diferença custa o QUADRADO — é a conta que se escolhe que decide o tamanho dos números.

**A MIGRAÇÃO DO SISTEMA (Qz: 64 → 32 bits), e o que mudou foi sempre O MESMO:** comparar
em vez de FORMAR. Em quatro sítios a pergunta era uma comparação e a resposta vinha de uma
construção — |a−b|<ε construía a diferença (multiplica os denominadores), sinal(x²−2)
construía x² (o dobro dos dígitos), o módulo de Cauchy iterava a órbita, Σ1/n^p acumulava
a soma. O par decide as quatro exactas. **A definição não mudou; mudou a conta escolhida.**

**E o guarda mudou de sítio:** antes ADIVINHAVA o tecto (comparava com um número meu) e só
depois construía. Com o tipo menor o guarda largo dizia «cabe», o racional grampeava, e
restava o CADÁVER da conta a passar por resultado. Agora PERGUNTA À OPERAÇÃO — o `qz`
conta o que não coube (`qz_saturou`), e a detecção está dentro da conta.

**E nunca se compara contra um valor saturado:** a partir do índice em que o termo não
cabe, o que se lê é o grampo. O varrimento pára no último índice HONESTO
(`cy_teto_honesto`), e esse índice diz-se. Medido: a órbita de Möbius é honesta até n=23.

**O preço, dito:** 32 bits saturam ao DOBRO da profundidade de 64 — não ao infinito, que
era a ilusão do tipo largo. A migração tornou o tecto VISÍVEL e metade; o defeito nunca foi
o tecto, foi o tecto silencioso.

**A PONTE ENTRE AS FACES, E O QUE ELA GANHOU:** a redução ℚ → ℙ¹(𝔽₁₂₇) é um
HOMOMORFISMO, logo uma identidade que vale em ℚ tem de valer nela. Donde **𝔽₁₂₇ NÃO PROVA
nada sobre ℚ — mas REFUTA**, sobre TODOS os casos, em milissegundos, sem um ramo e sem
nada crescer. É o refutador exaustivo que a casa não tinha. Controlos: a identidade
verdadeira nunca é refutada (0/1681), a falsa cai em 1600/1681.

E o LIMITE, com testemunha: **1 e 128 são o mesmo ponto em 𝔽₁₂₇ e não são o mesmo
racional** — coincidir na redução não é ser igual. Provar continua a ser trabalho de ℚ;
desmentir passa a ser de 𝔽₁₂₇, que é onde é barato. E a redução é TOTAL só por causa do ∞:
quando 127 | q dá o POLO, não um erro.

**O REFUTADOR COMO SEGUNDA TESTEMUNHA PERMANENTE.** As identidades da casa — centro,
membrana, Cayley-Hamilton, Lagrange, det(AB)=detA·detB — passam pelo refutador ANTES de
serem afirmadas, exaustivamente nas quatro variáveis sobre CINCO primos (129749
atribuições por identidade). Nenhuma cai; e as cinco MUTAÇÕES (sinal trocado, termo em
falta, + no lugar de ×) caem todas, com o primo e os valores ditos.

**Vários primos porque um só tem acidentes de característica:** (a+b)² = a²+b² é falsa e
PASSA em 𝔽₂, porque o 2ab desaparece. E exaustão compra-se BAIXANDO o primo: 13⁴ = 28561
contra 127⁴ = 260 milhões.

**A divisão de trabalho:** o Gato organiza as duas faces; a face exacta PROVA; a face
finita CAÇA O CONTRA-EXEMPLO. *A matemática continua grande; o detector de erro fica
pequeno.*

`tests/refuta.c` (5:0) · `tests/reducao.c` (6:0) · `tests/migracao.c` (5:0, equivalência em 213840 casos) · `tests/aritmetica.c` (6:0) · `tests/exaustao.c` (7:0) · `tests/sem_ramo.c` (6:0) ·
`tests/projetiva.c` (6:0). No Universal: `thm:fibra-por-andar`, `thm:sem-ramo`,
`cor:exaustao`, `cor:zero-infinito`. Ver [[project-teorema-do-gato]],
[[feedback-saturacao-nao-e-resultado]], [[feedback-o-write-diz-updated]].
