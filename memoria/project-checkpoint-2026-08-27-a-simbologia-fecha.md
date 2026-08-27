---
name: project-checkpoint-2026-08-27-a-simbologia-fecha
description: "27/08 — a simbologia passa a sair de teoremas, o terceiro custo paga-se com um bit, o infinito é o parceiro do zero, e a estrela é pental; 21 commits, 177 → 198 páginas, e a varredura apanhou trinta palavras que eu próprio tinha destruído."
metadata:
  type: project
---

# 27/08 — A SIMBOLOGIA FECHA, E A ARANHA COME O ANDAIME

De `9c44865c` a `2fd8b116`. `fisica.tex` de **177 para 198 páginas**, zero erros, zero
referências por resolver. Oito medidores novos, todos verdes.

## O que se construiu

**A simbologia deixou de ser dicionário.** As oito relações saem da dobra aplicada ao
espaço dos pares: o vinco da transposição dá o `=`, os blocos de dois dão o `≠`, a
orientação custa um bit por bloco e dá `<` e `>`, e juntar o vinco dá `≤` e `≥`. Fecham em
`2³` com a nula e a total.

**O terceiro custo estava por pagar, e paga-se com um bit.** A escada dos custos facturava
três — bloco, bit, coerência entre blocos — e o terceiro vivia numa meia-frase enterrada
numa prova sobre outra coisa: «e a ordem resultante é total». Nos dois símbolos há
exactamente um bloco fora do vinco; orientá-lo é a cláusula (5) executada, e é a única vez
que a obra escolhe lado. Depois a comparação pela posição mais alta herda esse bit, e é
total, antissimétrica e transitiva por indução. **Um bloco, um bit, uma indução.**

**A antissimetria é o que a dobra nunca fixa.** Sobre as funções do par ela tem dois valores
próprios: o directo fixa todos os pontos, o cruzado não fixa nenhum. Nenhuma grandeza
antissimétrica não-nula é invariante, porque obriga `2c=0` — que é a equação do vinco com a
soma posta a zero. A potência lê a paridade, e o menor invariante é o quadrado: o
discriminante. **O simétrico guarda o módulo e perde a direcção**, e é isso que torna a coisa
reversível sem a orientar.

**O infinito ganhou chão, duas vezes.** Lido na dinâmica é a metade sem ponto fixo — passo, e
não lugar. Lido no par é `τ(0) = [1:0]`, e o terceiro degrau exclui o denominador nulo por
construção. **Zero e infinito são as duas pontas de um bloco**, e nomear a segunda ponta não
acrescenta objecto.

**A meta-indução não é regra nova: é a mesma, dividida pelo termo.** A indução age no termo;
lida na razão — o terceiro degrau — a mesma igualdade vira «um mais o inverso da anterior».
A indução não fecha; a meta-indução fecha na estrela. E é reversível porque o determinante
vale um em módulo: subir e descer são o mesmo caminho.

**Cardinalidade é idempotência; ordem é potência.** E há preço: um idempotente não trivial é
um divisor de zero, pelo que **partir em duas metades fabrica divisores de zero** — e é por
isso que a ordem não vive desse lado.

**A estrela é pental, e o pental é o que a rede exclui.** `Δ=5` não é quadrado (logo o ponto
fixo sai do andar) nem potência de dois (logo a duplicidade não a conta). E o lema do cristal
já provava que uma rede admite `1,2,3,4,6` e nunca `5`. **Estava tudo no livro e nunca tinha
sido ligado.**

## O que se pagou

Vinte e uma dívidas, e quase todas minhas de sessões anteriores. Catorze citações punham na
definição do operador coisas que ela não diz — a pior na primeira tabela que o leitor vê.
Nove meias-verdades usavam a efectividade sem a nomear. O `≡` tinha quatro sentidos e passou
a ter um. As três dívidas de símbolo do quadro **não eram dívidas: eram citações por fazer** —
o `∈` estava fundado na definição do objecto, o `Π` no teorema do trial.

**E a fórmula-mãe do passo era falsa duas vezes**, medido: o domínio é o produto e não a
soma, e as duas cópias encontram-se no vinco em vez de serem disjuntas.

## O erro que mais me ensinou

A varredura do `≡` apanhou a sequência `equiv` **dentro de palavras**: `equivalência` virou
`=alência` em trinta sítios. **O compilador não podia apanhar isto** — é texto corrido — e
passou por três compilações limpas e dois commits. Só apareceu ao ler uma linha por outro
motivo.

E duas vezes um varredor meu usou inteiros de 32 bits e transbordou: uma deu sete falsos
quadrados, outra deu cinco onde o teorema prova que não há nenhum. **Se eu tivesse escrito o
segundo, tinha posto no livro uma contradição com um teorema meu de três sessões antes.**

> **Uma substituição global que compila não está verificada. Só está verificada quando se
> conta o antes e o depois.**

## A frase que fecha

> **A teoria não terminou quando acabaram os corpos. Terminou quando a própria operação que
> os construiu fechou sobre a sua dualidade.**

## Em aberto

Os empréstimos **declarados** das Partes de física — Carnot, a continuidade, a aceleração —
que entram marcados e não alimentam o alicerce. E a álgebra linear: `det`, traço, companheira
e ciclotómicos fazem trabalho portante sem serem construídos.
