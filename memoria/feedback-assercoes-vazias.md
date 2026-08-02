---
name: feedback-assercoes-vazias
description: "A asserção que passa sem poder falhar — OITO formas dela, e como reconhecer cada uma antes de commitar"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T17:36:04.756Z
---

# A asserção que passa sem poder falhar

Em 01/08/2026 escrevi **oito** famílias de asserções que davam verde sem medir nada. Não é
distração: é um modo de falhar meu, e tem formas reconhecíveis. Uma asserção vazia é **pior que uma
que falha** — a que falha avisa; a vazia conta-se como prova.

## As oito formas

**1. A constante disfarçada.** `ok("o quadro fecha", 1 == 1)`. E a variante mais difícil de ver:
`int cobertas = 6, semCorolario = 0;` seguido de `ok(..., semCorolario == 0 && cobertas == 6)` —
variáveis que eu próprio fixei duas linhas acima.

**2. A tabela literária.** Uma asserção que compara **strings que eu escrevi** contra strings que eu
escrevi (`strcmp(t[k].g, t[k].dd)` numa tabela de nomes de grupos). Falhou pela grafia
(`"T (o S¹)"` vs `"T"`) — mas o problema real é o oposto: **se eu errar o nome nas duas colunas,
ela passa**.

**3. O número escrito de cabeça.** `ok(..., cobertas == 6)` quando eram 7. Aqui o medidor apanhou —
mas a correção certa não foi trocar 6 por 7: foi medir *a frase que a asserção afirma*
(`coberta ⟺ n par OU n ∈ {1,3,7}`), para não haver número à mão nenhum.

Esta é a que mais se repete. **Três vezes em 01/08:** `"6 cobertas"` (eram 7); `"menos de 0,01%"`
(era 0,0333% — e eu nunca calculei, escrevi o número que *soava* pequeno); `"o salto do raster é
N−1"` (é N, porque o retorno anda N−1 em x **e mais 1 em y**).

**Em 01/08 (noite) foi a QUARTA vez do dia**, no `tikz.c`: *"3 instruções"* (eram 2), *">50
instruções"* (eram 40). E o agravante: **este memory já dizia tudo isto e eu não o consultei antes
de escrever.** É o mesmo que o [[feedback-simulacao-nao-bate]] regista — *escrever não impede,
CONSULTAR antes de escolher é que impede.* O gatilho a treinar: **sempre que a asserção contém um
literal numérico que eu não calculei ali mesmo, parar e perguntar de onde ele veio.**

**O antídoto é sempre o mesmo, e não é escolher melhor o número: é medir a LEI.** Em vez de
`saltoR == 2*N*(N-1)` para um `N` só, correr `N = 3..10` e exigir a fórmula em todos. Em vez de
`variação < 1e-4`, medir que a sensibilidade fica dividida por `(1+A'β)`, que é exato e não pede
limiar. **Uma lei medida em vários pontos não tem onde eu enfiar um palpite; uma constante tem.**

E há uma variante do antídoto que vale guardar, do `tikz.c`: quando a afirmação é *"X não afeta a
contagem"*, **medir a mesma entrada COM e SEM X e exigir igualdade** — aí não há número nenhum na
asserção, e ela mede exatamente o que diz.

**4. O caso degenerado que iguala os dois lados.** A pior, porque parece um teste a sério. Escolhi
`E` em senos e `B` em cossenos para medir o Poynting; são ortogonais, logo `S = 0` **exato**. E com
`S = 0`, as asserções *"preserva"* (`S → S`) e *"inverte"* (`S → −S`) são **a mesma afirmação** —
as duas passaram verdes. Ao pôr `S ≠ 0`, as duas falharam **e revelaram um erro de modelo que eu
nunca teria visto**: usava o produto interno onde o Poynting é o vetorial.

**5. O caso de teste escrito a partir do que eu ESPERO, não do que estou a medir.** Escrevi uma
asserção sobre acentos (`strstr(saiu, "cora\xE7\xE3o")`) sobre um fonte de teste que eu próprio
digitara **sem um único acento** — `"Acentos: coracao, area, tres, voce"`. A asserção nunca podia
passar, mas o dano é o simétrico e pior: se eu tivesse escrito a asserção *também* sem acento, ela
passava verde **sem que um só acento tivesse sido testado**.

**Aconteceu TRÊS vezes no mesmo dia**, sempre com acentos, e a terceira foi já dentro da unidade de
regressão que eu estava a escrever *para o bug dos acentos*. A causa não é o acento: é eu compor o
caso de teste a partir da minha ideia do assunto, em vez de o compor a partir **do defeito
concreto**. O fonte de teste que apanha o bug do cifrão tem de ter um cifrão desirmanado **e uma
palavra acentuada depois dele** — se faltar qualquer um dos dois, não há asserção que o veja.

**Antídoto: escrever o caso de teste a partir do fonte REAL onde o defeito apareceu**, copiando o
trecho que falhou, em vez de inventar um exemplo do mesmo género.

## E a irmã ao contrário: a asserção AMARRADA À RÉGUA

Não é vazia — mede. Mas mede a **régua** em vez do facto, e por isso quebra quando a régua muda,
mesmo com o facto intacto. Ao ligar a curva ao `tex.c`, **três** das minhas asserções caíram, e as
três fizeram bem em cair:

- `largura('W') == 944 && largura('i') == 222` — valores absolutos da tabela. **Um valor absoluto
  amarra a asserção a UMA fonte de medida.** O que eu queria afirmar não era *"o W mede 944"*, era
  *"o W é muito mais largo que o i"* — e a **proporção** sobrevive à troca da régua.
- `curva == tabela` exato, quando o medidor ao lado media com tolerância de 1. Não era discordância
  entre as duas: era o **arredondamento do meu próprio conversor**, que eu negava ao exigir
  exatidão. Medir o arredondamento é melhor que fingir que não existe.
- `distintas >= 20` deu 19. **Baixar para 19 seria escolher a constante outra vez.** O critério que
  ficou é a **razão entre o mais largo e o mais estreito** (5,34) — vale 1 se a régua não medisse
  nada, e isso não se escolhe.

**E o aviso que vem com isto:** inventei **três leis** sobre a mesma tabela de larguras e a medida
derrubou as três (*"a negra é mais larga"*: o `W` é igual nas duas; *"a negra nunca é mais
estreita"*: o `@`; *"nada visível é mais estreito que o espaço"*: o apóstrofo é 190 contra 277).
**Sobre uma tabela publicada não se afirmam leis — mede-se o que ela faz.** Se eu quero uma lei,
ela tem de vir do objeto, não do meu senso do que seria arrumado.

**6. ANOTAR o defeito em vez de o CORRIGIR.** Em 01/08 escrevi `ok("...", 1)` e, ao lado, o
comentário *"a asserção acima não mede nada"* — e deixei as duas coisas no ficheiro. **É pior que
não ver o defeito:** cria a aparência de rigor sobre uma medida vazia, e quem ler a nota assume que
foi tratada. *Ou corrijo na mesma edição, ou não escrevo a nota e deixo o defeito falhar.*

**7. O limiar posto no VALOR EXATO.** `V > 1e-3` para uma grandeza que dá exatamente `1,00e-3`. Um
limiar no valor exato não mede o fenómeno — mede o arredondamento, e falha ou passa por acaso. Se a
afirmação é sobre ORDEM DE GRANDEZA, comparar com a **escala** (*mil vezes o microvolt*), não com o
número.

**8. O ABSURDO NO RELATÓRIO que a asserção não apanha.** No `liga.c` o relatório imprimia
*"reflexão de 159,5%"* — **impossível num material passivo** — e a bateria estava verde, porque
nenhuma asserção olhava para aquele número. Duas causas somadas: o ramo da raiz dava `Re(Z)<0`, e um
`printf` partido em dois ficara **sem os argumentos** (o valor era lixo da pilha).

*Uma asserção verde não certifica o que ela não mede.* **Gatilho: ler os números impressos como se
fossem de outra pessoa, e perguntar se algum é fisicamente impossível** — percentagem acima de 100,
eficiência acima de Carnot, norma negativa, probabilidade fora de [0,1]. E compilar com `-Wformat`,
que apanha o `printf` sem argumentos e que eu não tinha ligado.

## Como aplicar

1. **Antes de commitar uma asserção, perguntar: que valor de entrada a faria falhar?** Se não
   houver nenhum, ela não mede. Se o único for "eu ter escrito outra coisa na linha de cima",
   também não.
2. **O caso de teste tem de ser não-degenerado, e isso mede-se.** Pôr uma asserção explícita sobre
   o pré-requisito (`ok("Σh é NÃO NULO — senão a seguinte mediria o vazio", ...)`) — assim o
   degenerado não pode entrar em silêncio.
3. **Nunca comparar o que eu escrevi com o que eu escrevi.** Tabelas de nomes, listas de
   classificação e constantes de cabeçalho são *citação*, não medida — marcá-las como citadas e pôr
   a asserção sobre um objeto construído.
4. **Quando um valor não bate, perguntar primeiro se o modelo está certo** — vale mesmo quando a
   falha parece um detalhe de sinal. Duas vezes nesta sessão a falha era da operação, não do sinal:
   escalar onde era vetorial, e "inverte as duas" onde cada uma inverte só uma.

## A NONA, de 01/08 de madrugada: **a asserção de unicidade com o crivo FRACO**

Escrevi `ok("há exatamente DUAS involuções que conservam a norma", ...)`. **São dez.** A asserção
caiu — e o defeito não era o número, era o **crivo**: conservar a norma é fraco demais. O crivo
certo era *respeitar o produto*, e com ele há uma só (Galois em grau 2).

**O sinal:** quando afirmo que *só existe um* de alguma coisa, a pergunta não é "quantos contei?"
mas **"o meu filtro é apertado o suficiente para a unicidade ser verdadeira?"**. E a correção ficou
ESCRITA no medidor — `fecha.c` §F2 mede as dez ao lado da uma, para o crivo se ver.

## A DÉCIMA: **a secção inteira definida e nunca chamada**

Acrescentei `secao_S6` ao `smartcontract.c` com quatro asserções, e o `main` nunca a chamou — o meu
`replace` não bateu por causa de espaços. **As quatro asserções não falharam: desapareceram.** A
bateria teria dito "7 unidades, 0 falhas" e estaria certa.

**Quem apanhou foi o `-Wunused-function`.** É o irmão exato de *"um medidor que não compila não
falha, desaparece"* ([[feedback-dois-caminhos]]), agora um nível abaixo: dentro de um medidor que
compila e passa. **Depois de acrescentar uma secção, confirmar que a contagem de asserções SUBIU.**

**E o número de cabeça voltou pela sexta vez** no mesmo dia (contei quatro instruções onde havia
cinco). O antídoto continua a funcionar: **medir a LEI em vários pontos** — `bytes = Σ(1+operando)`
em cinco programas — em vez de acertar melhor no número.

## A DÉCIMA PRIMEIRA, e é a IRMÃ DE TODAS: **a asserção que nunca PASSA**

O Aarão perguntou: *"qual o problema desses dois medidores que não falharam nem passaram?"*

O `ancora.c` e o `homogeneo.c` provam teoremas **negativos** — que a soma-de-palavras não é a
tradução (0,14% e 11,15%). O resultado é verdadeiro. Mas eu afirmava-o **ao contrário**: a asserção
dizia que a rotação *fecha*, e como não fecha ela falhava sempre. E o `bateria.sh` tinha uma
**lista à mão** (`case ancora|homogeneo`) que traduzia a falha em "NEGATIVO, teorema por projeto"
e calava.

**O preço mediu-se:** trunquei o corpus a **três pares** e o `ancora` deu exatamente o mesmo
veredito que com 196 415. O medidor não distinguia *"o teorema negativo confirma-se"* de *"os dados
evaporaram"*.

> **Uma asserção que nunca passa é tão vazia quanto uma que nunca falha.**

É a irmã das 66 `ok(...,1)`, do outro lado do espelho. E o sintoma estava **à vista no relatório**,
em duas colunas que eu lia sem ler: *"2 negativos por projeto"*.

**O conserto — dizer o negativo POSITIVAMENTE:**

```c
ok("a taxa de fecho fica ABAIXO de 5% — o Σ não é a tradução", taxa < 5.0);
ok("o corpus tem pelo menos 10 000 pares — o negativo é sobre DADOS", n >= 10000);
```

Agora passam e **podem** falhar: se a taxa subir (seria um achado) ou se os dados sumirem.

**E a regra geral:** *toda isenção numa bateria é uma lista à mão, e toda lista à mão acaba a
desculpar o que devia medir.* Um medidor com direito a falhar não é medido. A categoria inteira
foi-se — 232 de 232 verdes, zero isenções.

## A DÉCIMA SEGUNDA: **o critério que JÁ É VERDADE por construção**

Pedi ao modelo que a resposta dele tivesse espectro **conjugado** do da frase — e para um sinal
**real** isso já é verdade sempre: `F(N−k) = conj(F(k))`. **O resíduo não tinha para onde descer.**
Iterei seis vezes e ele mexeu 2,4% *por ruído*, e eu quase li isso como "quase convergiu".

**O sinal:** antes de otimizar contra um critério, perguntar **que entrada o violaria**. Se nenhuma
puder violá-lo, ele não é um alvo — é uma tautologia com um número ao lado. É a irmã da asserção que
não pode falhar, agora no papel de **função objetivo**.

O critério certo era a decomposição par/ímpar, que **não** é automática — e com ele o laço passou a
dizer alguma coisa (fechou em período 2).

Ver [[feedback-dois-caminhos]] — a mesma família: uma asserção mede um caminho contra um valor que
eu escrevi, e se eu errei a pensar ela confirma o meu erro.
