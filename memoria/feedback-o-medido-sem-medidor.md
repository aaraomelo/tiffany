---
name: feedback-o-medido-sem-medidor
description: "o par dual da invisibilidade na bateria: o \\medido que não nomeia programa (28 blocos), e o medidor que existe mas é citado só num .tex que a varredura não lê (banco/fala.c)"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-07T05:56:59.173Z
---

07/08/2026. A bateria conta **319 : 319 verdes** e essa contagem está certa — mas ela só vê o que é
**citado**. Um bloco `\medido{...}` que não nomeia ficheiro `.c` nenhum está fora do universo que ela
percorre: não é uma referência quebrada (não há referência), não é um medidor que falha (não há
medidor). É invisível por construção.

São **28**: 18 em `teoria.tex`, 10 em `catalogo.tex`. Os dois papers (`corpo-estelar`, `dualsort`)
estão limpos — zero.

## Como se acham

```python
# localizar cada \medido{...} contando chavetas, e ver se o bloco cita .c ou .py
```
Não serve `grep '\medido'` sozinho: os blocos têm várias linhas e chavetas aninhadas.

## O que NÃO funciona para os recuperar

Tentei cruzar os números afirmados com as saídas dos medidores em `/tmp/bateria/`. **Falhou**, por
duas razões que valem para a próxima vez:

1. **Falsos positivos por tamanho.** `101`, `144`, `200`, `400` aparecem em quase todas as saídas. Só
   números com 4+ dígitos e pouco redondos discriminam.
2. **Falsos positivos do meu parser.** O regex apanhava `$k=1,3,5,7,9$ dão $n=1,4,11,29,76$` e
   colava tudo em `13579` e `14112976` — números que não existiam em lado nenhum, e que eu ia
   reportar como órfãos. Ao extrair números de LaTeX, **as listas separadas por vírgula não são um
   número**.

Verifiquei os quatro candidatos mais distintivos um a um (`3240`/`1522` contra `gerador.txt`, `6348`
contra `dualcifra.txt`, `6561` contra `dna.txt`, `100001` contra `pi_rei.txt`): **todos coincidências**.
Num caso o número aparecia, mas o outro número do mesmo bloco não — e um bloco só se confirma se
**todos** os seus números aparecerem.

## O que isto quer dizer

Não são necessariamente falsos. Muitos parecem resultados reais de sessões antigas. Mas são
**inauditáveis**: afirmam número e resíduo, e não há como voltar a correr. E são exatamente do tipo
de frase que já me traiu — *"resíduo 0"*, *"sem uma única exceção"*, *"zero discordâncias"*.

A legenda do próprio catálogo (`catalogo.tex`, na definição de `\medido`) promete: *«o enunciado foi
verificado por programa (…) **e o medidor vai indicado**»*. Vinte e oito não cumprem a promessa que a
legenda faz.

**Para resolver:** por cada um, ou achar o medidor e citá-lo, ou escrever o medidor, ou retirar a
afirmação. Não há quarto caminho — e deixá-los como estão é o que os faz passar por medidos.

Relacionado: [[feedback-o-medidor-que-nunca-mediu]] (a atestação guardava o resultado e não o motivo),
[[feedback-assercoes-vazias]], [[feedback-dois-caminhos]].


---

## O DUAL, 18/08/2026: o medidor que a varredura não lê

O de cima é *afirmação sem medidor*. **O outro lado é medidor sem varredura** — e apanhei-o hoje.

A bateria dava **507**, e o repo tinha **508**. Não falhou nada: `banco/fala.c` existe no disco, está
citado, e a bateria nunca o viu — porque a única citação está em `corpus/docs/dualsort.tex:666`, e a
varredura lia só cinco alvos (`teoria`, `catalogo`, `enredo`, `papers/`, `conecthus/`).

**E o comentário da própria bateria já dizia que isto ia acontecer**: *«corpus/docs/medida.tex foi o
primeiro»*. A correcção da altura acrescentou `papers/` e `conecthus/` — e não acrescentou o
`corpus/docs/` que o próprio comentário nomeava. Terceira vez.

### Como se acha (barato, e devia correr sempre)

```bash
# a lista larga (todo .tex do repo) contra a lista que a bateria varre
comm -13 <(lista_da_bateria) <(git grep -ohE '(tests|banco)/[a-z_0-9]+\.(c|py|js)' HEAD -- '*.tex' | sort -u)
```
Um nome nesse `comm` é um medidor que morre calado. Cuidado com o diagnóstico largo: procurei pelo
*basename* `fala` e o `git grep -l` devolveu 34 ficheiros, nenhum deles a citação real. **O grep de
diagnóstico tem de ser tão literal quanto o da bateria.**

### A frase que vale para os dois lados

Um medidor pode morrer de dois modos, e **nenhum dos dois faz o total descer**: ou a afirmação não
nomeia programa, ou o programa não é lido por quem conta. Por isso a regra é ler **as duas listas**,
nunca só o total — que é a mesma lição de [[feedback-o-exit-sombreado]] e
[[feedback-dois-caminhos]], e o teste obrigatório de [[project-tres-documentos]].

Relacionado: [[feedback-o-medidor-que-nunca-mediu]], [[feedback-procurar-na-bateria-antes]].

## A TERCEIRA FACE (22/08): a afirmação que nem `\medido` tem

As duas de cima são sobre o BLOCO — o `\medido` sem programa, e o programa sem
citação. Falta a que não tem bloco nenhum: **a frase que afirma um facto e à
volta da qual não há `\medido` a escrever**. Ela não aparece em contagem
nenhuma, porque não há nada para contar.

Três, achadas por leitura e medidas nesta sessão — todas de papers que a casa
cita como fundamento:

- «a ressonância é a raiz dupla outra vez» (catálogo) → a ordem que o
  `edo_particular` devolve É a multiplicidade do autovalor. §W73.
- «o zero invariante é o **ponto fixo** $x^*$, onde o fluxo PARA»
  (`broca-so/papers/equacoes_diferenciais.tex`) → para $\dot x = Ax$ isso é o
  NÚCLEO, e `posto + dim ker = n` passa a ser «o que se move + o que fica
  parado». §W77.
- «o conjunto das soluções não é um espaço vetorial, é um espaço vetorial
  TRANSLADADO» (idem) → o particular mais $t$ vezes o núcleo, e o caso em que
  não há equilíbrio nenhum. §W78.

- «o invariante lê-se no fluxo como a **conservação da norma** pelo esquilo»
  (idem) → e o porquê é uma linha: `xᵀAx` é escalar e igual ao seu simétrico
  quando A é antissimétrica. A forma geral é mais forte — `xᵀAx = xᵀSx` para
  QUALQUER A: a energia **não vê** a parte antissimétrica. §W79.

**How to apply:** ao ler um paper da casa para trabalhar nele, marcar as frases
que **afirmam** e não trazem `\medido`. São candidatas a bloco novo, e as quatro
de cima deram quatro — todas com gume, e uma delas (o sistema que nunca para)
destapou um significado que a álgebra sozinha não mostrava.
