---
name: project-checkpoint-2026-08-16-a-raiz-sai
description: "16/08 — a raiz e o transcendental saem por TEOREMA, não por gosto; e das 1006 chamadas só 17 alimentavam asserções"
metadata: 
  node_type: memory
  type: project
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-17T00:46:38.482Z
---

16/08/2026, 23 commits de `a8ed821` a `2298d0f`. Bateria **499:499 → 501:501**, dois
medidores novos (`tests/entrega.c` 9:0, `tests/cruzado_potencia.c` 6:0), `geometrico.tex`
32 → 34 páginas com três teoremas novos.

## O TEOREMA QUE ORGANIZA O DIA

O Aarão: *«um produto cruzado antissimetrico é invariante a potencias … raiz é o inverso,
tudo sai inteiro quando normalizado»*. Procurei nos papers e **não estava formalizado** —
mas também **não era novo**: era a linha `det M_k = (−1)^k` da tabela do §sec:euler, dita
ali como observação sobre o determinante. **O determinante É o cruzado** (a área), e
aquela linha já era esta invariância sem a enunciar.

`thm:cruzado-potencia` no geometrico:

```
(1)  Cruz(Au,Av) = det(A)·Cruz(u,v)          da multiplicatividade do det
(2)  Cruz(A^k u, A^k v) = (det A)^k·Cruz(u,v)   e o passo não menciona k
(3)  |det A| = 1  ⟹  |Cruz| INVARIANTE na órbita
(4)  Dir² + Cruz² = N(u)·N(v)                Lagrange: cos²+sin²=1 sem formar nem um
(5)  e a FRONTEIRA, que a medida obrigou a escrever
```

**A (5) eu não previ.** Escrevi que as perguntas métricas seriam todas invariantes, e a
medição corrigiu-me: **PARALELOS (Cruz = 0) atravessa a órbita; PERPENDICULARES (Dir = 0)
cai já no primeiro andar.** `A_m` tem |det| = 1 mas NÃO é ortogonal — preservar a área não
é preservar a métrica. É a assimetria Dir/Cruz lida na ACÇÃO, e é ela que dá conteúdo à
palavra «invariante»: se tudo atravessasse, nada estava a mexer.

## A PERGUNTA QUE MUDOU O TRABALHO TODO

Andei a caçar raízes uma a uma até fazer a análise certa: **para cada `sqrt`, ver se a
variável que a recebe aparece nalgum `ok(...)`**.

```
186   só apresentação ou intermédio que acaba num printf
 62   expressão solta
 19   ALIMENTA UMA ASSERÇÃO      ← só estas
```

**O número nunca foi 272. Era 19.** E uma raiz que só imprime é a representação a
fazer-se no fim, que é a regra da casa e não uma excepção a ela. Aplicada aos
transcendentais: 729 chamadas, **18** alimentavam asserções. No fim do dia, das **1006**
chamadas a raízes+transcendentais restam **17** a alimentar asserções, e todas legítimas
(`pow(x,x)` é a função sob estudo; `1/√dim` é a definição da atenção; `exp` é a lei
física; `sin/cos` a gerar os DADOS).

**Como aplicar:** antes de migrar N ocorrências de uma coisa, classificá-las pelo que
ALIMENTAM. O trabalho é sempre uma fracção pequena, e o resto é a fronteira legítima.

## O QUE A RAIZ ESCONDIA — sete achados do mesmo tipo

A função transcendental estava quase sempre lá **para ser cancelada contra si própria**,
ou para atravessar uma desigualdade que a monotonia já atravessava. Quando saiu, ou a
asserção passou a poder falhar, ou apareceu o inteiro que ela escondia:

| onde | o que estava | o que era |
|---|---|---|
| `octeto` | `acos(ip/(n0·n1))` vs `acos(−1/3)` | `ip/(n0n1)` **É** −1/3: dois acos a mascarar x = x |
| `octeto` | `acos(cos(2π/3))` vs 120 | acos∘cos, a função e a sua inversa |
| `koch` | `log(4^N)/log(3^N)` vs `log4/log3` | os N **cancelam** |
| `xx` | `log(xc·0,99)+1 < 0` | com xc = 1/e isso **É** log(0,99) |
| `milenio` | `L = log(exp(−λt))`, passo constante | log∘exp, e λ definido como log2/5730 |
| `gerador` | `√(L/C) − √((L/2)/(C/2))` | `(L/2)/(C/2)` **É** L/C |
| `dispositivo` | `produto = −1; if(produto == −1)` | a atribuição da linha de cima |

E os INTEIROS que apareceram por baixo:

- **Koch**: `1 < D < 2` **é** `3 < 4 < 9`. E a irracionalidade sai da factorização única —
  `D = p/q` daria `4^q = 3^p`, e 4 = 2² não tem factor 3.
- **Lamé**: a forma de 1844 é INTEIRA — `k` passos exigem `F_{k+2} ≤ n`. `F(14) = 377 ≤ 400`
  e `F(15) = 610` já passa. **É mais forte que a assintótica** `log(400√5)/log φ`, que dava
  14,1 de folga onde o medido é 12. A versão moderna é a aproximada.
- **Selberg**: `2·cosh(ℓ_m) = t₄(m)`, o TRAÇO. 7, 34, 119, 322, 727 — e o 7 é Lucas L₄.
  O decimal `1.9248473002` escrito à mão era esse inteiro visto por um logaritmo.
- **Tetraedro**: os vértices são INTEIROS, `⟨vᵢ,vⱼ⟩ = −1` e `‖v‖² = 3` nos SEIS pares.
  A equidistância era a tese e media-se UM par.
- **dB**: `10log(a) − 10log(b) > 1` é `(a/b)¹⁰ > 10`.

## AS CINCO PRIMITIVAS, e a inversão que não é uma operação

O Aarão mandou ler. `thm:derivacao-primitivas`: Soma, Multiplicação, Divisão, Dual,
Inversão **não são independentes** —

```
M + M† = tr M · I      o CENTRO
M · M† = det M · I     a MEMBRANA
M⁻¹    = M† / det M    a INVERSÃO
```

*«Cada operação, emparelhada com o dual, dá a sua parceira, e a parceira não entra na
lista.»* Pela mesma conta `a − b = a ⊕ b†` — e é por isso que são CINCO e não sete.

**Isto corrigiu-me a lib que eu tinha acabado de escrever**: a `rt_inversa2` fazia
`Inv[0] = M[3]/d` à mão, que é **a adjunta sem lhe chamar o nome**. Uma operação escrita
onde havia uma derivação é a lista a crescer sem razão.

E no geométrico a inversão é a TROCA `[p:q] ↦ [q:p]`, sem divisão e sem ramo. As duas
concordam pela derivação: `S† = −S`, `det S = −1`, e **os dois sinais cancelam** —
`S⁻¹ = (−S)/(−1) = S`. Escrevi «S é a sua própria adjunta» e é falso; o medidor apanhou-o.
`thm:inversao-dual` no geometrico.

## A LIB: de 37 para 61 funções

`rt_induz`/`rt_desce` (a indução e a descida, `thm:meta-inducao`) · o cruzado, o bivector
com as DUAS rotas, a ordem sem raiz, `rt_raiz_exacta` · **ℤ[√D]** (`rt_zd_mul`, `rt_zd_pot`,
`rt_zd_norma`, `rt_traco_metalico`) · a ENTREGA em fracção contínua · as INVERSAS
(`rt_raiz_k`, `rt_log_int`, `rt_adjunta2`, `rt_inversa2`, `rt_zd_conj`).

**A `rt_bivetor_soma` existe por uma razão que vai dita no header**: custa O(n²) onde a
`rt_bivetor2` custa O(n), e serve para MEDIR e não para calcular. Sem ela, verificar
Lagrange contra a forma fechada é comparar a definição consigo própria — que era o defeito
do §S3 do `semantico.c`.

**E o aviso das inversas**: medir `f(f⁻¹(x)) = x` NÃO MEDE NADA. Encontrado três vezes
num dia, sempre disfarçado por conversões que o tornavam ilegível.

## A ENTREGA É A PALAVRA

O Aarão: *«essa representação vai até ao fim e entrega assim mesmo em long int»*. Um
decimal escrito NÃO é aproximado: «3.14159» É 314159/100000. O que o estraga é o double —
`0,1` não é `1/10` em base dois, e não por imprecisão mas porque `10 ∤ 2^k`.

```
10000 decimais de três casas, ida e volta
pela PALAVRA:   10000 exactos (100%)
pelo DOUBLE:     9825 exactos  (98%)
```

O primeiro número sozinho não dizia nada; é o **par lado a lado** que é o motivo de
existir. E de «3.14159» saem **22/7 e 355/113** sem ninguém os pedir. `cor:entrega`.

A CF já estava em `lib/aritmetica.h` (`nt_fc`, `nt_convergentes`) — não escrevi a segunda
cópia; a `reta.h` ganhou só a PONTE (ler o texto, o sinal, reconstruir).

## OS MEUS ERROS DO DIA, e o que os apanhou

1. **Escrevi por cima do `tests/potencia.c`** (153 linhas, a potência como TERCEIRA
   operação — clone/reprodução/potência, com Pisano). **Terceira vez.** O Write disse
   «updated» e não li; o `git ls-files` da minha própria regra não fiz.
   **O que o apanhou foi O TOTAL DA BATERIA NÃO SUBIR** — 500 quando tinha de ir a 501.
   Nenhuma asserção podia ver isto. Ver [[feedback-o-write-diz-updated]].
2. **Quatro contagens de cabeça**: 38416 onde eram 50625 (15⁴), 59535 onde eram 72171
   (11·9⁴), «o 9 está na gama 13..19», «mil pontos» onde eram 2901. **O conserto não é
   trocar o número: é escrevê-lo como EXPRESSÃO** (`11L*9*9*9*9`), que o compilador calcula.
3. **Três transbordos**: `1000*(tot − so_dir)` com tot ~6e17; `(agora_ns()−t0)/reps` a
   truncar a zero; e o `1000·tot` outra vez. **Divide-se primeiro.**
4. **Duas asserções vazias escritas por mim** enquanto corrigia asserções vazias:
   `if(w*h != h*w)` e «o q medido não excede o DOBRO de 1/σ²» (passava por 0,142 ser
   pequeno, não por bater).
5. **Duas previsões erradas que deram resultados melhores**: a perpendicularidade NÃO é
   invariante (é a assimetria Dir/Cruz) e o cancelamento do ω₀ NÃO é bit a bit (180 de 196,
   um ULP) — logo a versão sem raiz é a EXACTA, e não só a mais barata.

## O ESTADO

doubles **3553** em 122 ficheiros: grupo A 184 (quase todo justificado — `libc.c` É a
conversão para o cliente, `traduz.c` é o compilador, `limiares/po_corpo/circuito_tradutor`
têm o double como CONTROLO), B 556, C 2813. `sqrt`/`hypot` 298 → 270. `tectos.py`: 74
candidatos P3 em 30 ficheiros.

Ver [[project-a-reta-construida]], [[project-o-fecho-do-dual-lagrange]],
[[feedback-assercoes-vazias]], [[feedback-a-referencia-escrita-a-mao]],
[[feedback-representacao-inteligente]].
