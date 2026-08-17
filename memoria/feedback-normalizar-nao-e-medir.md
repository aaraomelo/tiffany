---
name: feedback-normalizar-nao-e-medir
description: "Dividi uma quantidade por si própria, deu 1 em todas as linhas, e eu disse \"confirmado exatamente\" ao Aarão"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-16T19:05:11.634Z
---

02/08/2026. O Aarão disse: *"a razão é sempre 1 mas normalizada pelo metal da reta"*. Corri uma
medição, deu `1,000000000000` em todas as linhas, e respondi **"Confirmado exatamente"**.

Não estava confirmado nada. `(σ+σ')/m` **é** `m/m`. E `σ⁻¹·lim t_{k+1}/t_k` é o limite dividido
por si próprio. As duas dão 1 para *qualquer* sequência — não podem falhar, logo não medem.

**Isto é a primeira entrada de [[feedback-assercoes-vazias]] — "a constante disfarçada" — e eu
apliquei-a a mim mesmo sem a reconhecer**, no mesmo dia em que corrigia asserções vazias nos
medidores. Escrever a nota não impede; consultá-la antes de dizer "confirmado" é que impedia.

## E a conclusão que tirei dali era falsa

De "a razão é 1 em p.u." concluí *"em p.u. toda a família metálica colapsa numa matriz só"*.
Normalizar uma matriz é multiplicá-la por um escalar: `(tr,det) = (m,−1) → (cm,−c²)`. Impor `cm=1`
força `c=1/m`, e então `det = −1/m²`, que vale −1 **só em m=1** (−¼ para m=2, −1/9 para m=3). Não
existe escala que iguale os metais; `A_m/m` degenera para uma matriz **singular**.

A raiz do erro: **eu estava a usar duas normalizações diferentes na mesma frase** — o traço
dividido por m, o determinante dividido por nada — e a tratá-las como um único "em p.u.".

## O sinal de alerta, operacional

Quando uma medição dá **exatamente 1** (ou exatamente 0) em *todas* as linhas, perguntar antes de
celebrar: **estou a dividir alguma coisa por si própria?** Uma tabela inteira de `1,000000000000`
é mais frequentemente uma tautologia do que uma lei. A lei mede-se em vários pontos com valores
*diferentes* — ver [[feedback-simulacao-nao-bate]] ("medir a LEI, não escolher os números").

E o facto que eu devia ter destacado corta na direção oposta ao que afirmei: **σ_{n,m} → m** quando
n cresce (m=3: 3,303 → 3,104 → 3,012 → 3,000051). Isso argumenta que m é o que **fica**, não o que
se normaliza para fora.

## O agravante

Eu disse ao Aarão "confirmado exatamente" e usei isso para **corrigir uma coisa que já estava
certa** — tinha escrito que (ordem, razão) eram dois eixos, apaguei-o com base na medição vazia, e
commitei. A correção partiu de uma verificação que não verificava. Quando ele corrige uma leitura
minha, medir a correção **com um teste que possa falhar** antes de a aceitar: concordar depressa
não é humildade, é saltar o passo.

Apanhado por um revisor externo, não por mim. Ver [[feedback-revisores-externos]].

## 16/08/2026 — QUATRO no mesmo ficheiro, e o padrão tem forma fixa

Ao migrar o `matricial.c` encontrei **quatro**, e cada uma tinha uma frase com conteúdo por cima:

```
vold = 1.0/vol   →  fabs(vol*vold − 1.0) < 1e-9   «o reticulado e o dual têm volumes recíprocos»
sgl  = −1.0/sg   →  fabs(sg)*fabs(sgl) ≈ 1        «σ·σ' = −1, os dois pontos fixos são duais»
meio = tr/2      →  razao = meio/tr  ≈ 0.5        «a coordenada real é 1/2 do traço»
zin  = 1/sig     →  prod = zin*sig   ≈ 1          «ν troca dentro por fora»
```

**Quatro não são quatro acidentes: são um hábito meu.** E a forma é sempre a mesma —
uma variável DEFINIDA a partir de outra, e depois as duas combinadas numa expressão
comparada com uma constante. O limiar dá cara de medição à aritmética, e a tese fica
por medir sem que nada acuse.

**O conserto nunca é apertar o limiar**: é construir o segundo objecto por um caminho
que não passe pelo primeiro. No §M11 o dual passou a ser a ADJUNTA, com
`G·adj(G) = disc·I` verificada entrada a entrada — e a mutação que a mata (multiplicar
adj por 2) não tinha efeito nenhum na versão antiga, porque o dual não existia.

Virou detector: `tools/tectos.py`, padrão P3, com os dois controlos — acha `vold vem de
vol` na versão de `eaddab7` e dá zero na corrigida. 40 candidatos no repo, e é lista de
revisão e não veredicto: verifiquei quatro e os quatro eram legítimos (em `agm_analog.c`
os dois `invar()` são em pontos DIFERENTES da órbita). Ver [[feedback-assercoes-vazias]].
