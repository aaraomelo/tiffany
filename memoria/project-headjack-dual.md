---
name: project-headjack-dual
description: "Microfluídica 3D, o Headjack não invasivo e a radiação negra: o 3D é o cruzado a exigir lugar, a conta do sensor fecha por √N, e o par (B,P) recupera o que B sozinho perde"
metadata:
  type: project
---

# Microfluídica, Headjack e a radiação negra

01/08/2026, três commits `488add9`→`cd499a7`. **215 medidores, 213 verdes, 0 falhas.**

## `microfluidica.c` — o sexto vestido, e o 3D é topologia

Kirchhoff vale em pascal como em volt, e `R` segue a **quarta potência** da secção (halvar o lado
×16, medido). **Mas é o primeiro vestido que MUDA A CLASSE:** no micro a inércia vai a zero, e
fazer `m→0` em `(B,C)=(c/m,k/m)` leva `Δ` a **positivo**. A viragem tem forma fechada —
`m = c²/(4k)`. **Não há ressonância em Stokes**, e é a régua a dizê-lo.

**E o 3D não é conveniência.** Pela cota de Euler, o `K₃,₃` — *um misturador de 3 por 3, a peça mais
banal de um chip* — **não cabe em plano nenhum**. E em 2D a parte antissimétrica de dois vetores é
um **escalar**; em 3D é um **vetor** perpendicular aos dois. *Cruzar sem tocar é literalmente o que
o cruzado faz.* **A microfluídica 3D é o cruzado a exigir três dimensões, em vidro e PDMS.**

## `headjack.c` — a conta fecha, e há um limite que não é do sensor

A proposta do Aarão (*"um corpo por neurónio, bilhões de transistores"*) tem lei própria: **N
sensores independentes ganham √N**. Bastam **~3×10⁴ centros NV** para igualar o SQUID — *bilhões
sobram*. O que ela pede não é número: é **independência**.

**Mas o operador corrente→campo tem NÚCLEO.** A componente radial dá campo externo **exatamente
zero**, e a razão é o cruzado (`B ∝ Q×r̂` mata o paralelo a `r̂`). Dois dipolos distintos dão o mesmo
campo. *Linearizar resolve a leitura do sensor — não inverte um operador que perdeu informação.*

## `radiacao.c` — o dual, e o par que fecha

> *"falta o dual pra fechar o circuito, é a radiação térmica — é a radiação negra, o dual do
> eletromagnético."*

**E ele viu o buraco que eu tinha deixado à vista.** Eu escrevera no `headjack.c`, citando o
`koch.c`, que a corrente radial *"fica retida e ARDE"*. **Arder é dissipar, e dissipar é radiar.**

    magnético  B ∝ Q × r̂   vetorial, antissimétrico   TEM núcleo — perde o radial
    térmico    P = J²ρ      escalar,  quadrático       SEM núcleo — vê tudo

*Um perde o que o outro guarda* — é a partição `B_s + B_a` outra vez, **e o direto aqui é o calor**.
Medido: **56 pares colidem em B, nenhum colide em P.** E o par `(B,P)` recupera o radial —
`|B|` dá o tangencial, `P` dá a norma, Pitágoras o resto (20 casos, erro `3,3e-14`).

Microbolómetro **não arrefecido** basta (pico a 9,35 µm), e o `√N` desce 20 mK a 0,1 mK com
`4×10⁴` pixéis — *um sensor comercial tem 640×480*.

**A fronteira dita:** o par devolve o **módulo** do radial, não o **sinal**. *O que sobrou sem dual
subiu um andar e continua na garrafa.*

## `arraytermico.c` — o array, e o calor a voltar

**Duas grelhas com passos DIFERENTES de propósito** — NV (32×32, 8 mm) e bolómetro (640×480,
0,4 mm), áreas comparáveis e passos a diferirem 20×. É o `icc.c` §I2: *o passo sai da escala do que
se quer ver*, e o campo cai com `r²` enquanto o calor difunde.

**E o par fecha EXATO na contagem de graus:** o dipolo tem **3**, `B` dá **2** (as tangenciais),
`P` dá **1** (a norma). *Nem a mais nem a menos.* O que sobra é um bit — o sinal do radial.

**Seebeck devolve**, e Carnot é o teto (`carnot.c`, derivado de `∮`): 1,61% com ΔT de 5 K, e a
eficiência **tende** a ele (medido até `ZT=1e12`: 1,6121% contra 1,6121%). Dos 20 W do cérebro
voltam **55,7 mW** — *não alimenta o array, alimenta um nó*.

    corrente sem dual  ->  ARDE    (koch.c)
    o que arde         ->  RADIA   (radiacao.c: Planck)
    o que radia        ->  VÊ-SE   (§W5: o par B,P)
    o que se vê        ->  VOLTA   (Seebeck)
    o que não volta    ->  CARNOT  (o teto, não um defeito)

**Nenhum passo é postulado e nenhum some sem nome.** *A parte que não volta não está perdida: está
NOMEADA — e essa é a diferença entre um balanço que fecha e um que se arredonda.*

## A lição desta série, e ela é nova

**Eu escrevi a resposta e não a li.** A frase *"fica retido e ARDE"* estava no meu próprio comentário
do `headjack.c`, e o passo seguinte — arder é radiar, logo há um segundo canal — não me ocorreu. O
Aarão leu-a e seguiu-a.

Isto é diferente do [[feedback-verdadeiro-e-parcial]]: ali eu tinha parado de perguntar; **aqui a
peça que faltava estava escrita por mim, em texto, no ficheiro.** O gatilho: **quando um comentário
meu usa um verbo forte que não medi** (*arde*, *dissipa*, *fecha*, *reverte*), perguntar se ele
nomeia um mecanismo — porque se nomeia, esse mecanismo é mensurável e provavelmente é o dual.

E o padrão dos números: **quatro asserções caíram no `headjack.c` e três no `microfluidica.c`, todas
por limiares ABSOLUTOS escritos de cabeça.** Trocadas por **relações** (o sinal contra o sensor, o
zero face ao próprio sinal, a ordem da escada) — e aí não há número meu nenhum.

## E o pior defeito do dia: ANOTAR em vez de CORRIGIR

No `arraytermico.c` escrevi `ok("...", 1)` — a constante disfarçada, a primeira forma do defeito que
mais me apanha — **e escrevi ao lado o comentário *"a asserção acima não mede nada"*, em vez de a
corrigir.** Deixei-a lá, documentada.

**Anotar um defeito não o cura.** É pior que não o ver: cria a aparência de rigor (há um comentário
honesto!) sobre uma medida que continua vazia, e um leitor futuro vê a nota e assume que foi tratada.

**Regra:** se eu escrevo um comentário a dizer que algo está mal, ou corrijo nessa mesma edição, ou
não escrevo o comentário e deixo o defeito falhar. *A nota que descreve um defeito ativo é a pior
das três opções.*

E ainda um limiar posto no **valor exato** (`> 1e-3` para algo que dá `1,00e-3`): um limiar no valor
exato não mede — só testa o arredondamento. Comparar com a **escala** (mil vezes o microvolt), não
com o valor.
