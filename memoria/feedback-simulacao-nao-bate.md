---
name: feedback-simulacao-nao-bate
description: "Quando uma simulação não faz o que devia: a ordem de investigação é escalas, depois convenções, e só então a lógica — fui direto à lógica e perdi tempo"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T15:42:59.697Z
---

# Quando a simulação não faz o que devia

Em 01/08/2026, duas simulações falharam e nas **duas** o meu primeiro palpite foi *"a lógica do
controlo está errada"*. Nas duas vezes não estava. A ordem certa de investigação é esta, e é do
mais barato para o mais caro:

## 1. As escalas dos parâmetros fecham ENTRE SI?

No `motor.c` pus `dt = 2e-4` — o fluxo andava `1,3e-4` por passo, e uma volta completa pedia
~48 mil passos, mas eu simulava 6 mil. A máquina *"não girava"* porque **o meu passo era pequeno
de mais**, não por defeito do DTC. E, ao lado, o rotor tinha `ω = 12 rad/s` imposta — dezenas de
vezes mais rápido que o estator. Dois parâmetros que eu escolhi sem os comparar um com o outro.

**Fazer a conta de ordem de grandeza antes de correr:** quanto anda por passo × quantos passos =
quanto tem de andar? As constantes de tempo dos vários subsistemas estão na mesma década?

## 2. Os sinais e convenções estão consistentes?

Duas vezes no mesmo dia:

- `T_e = ψs × ψr` quando a fonte dá `T_e = −(3/2)P(Lm/Lσ)·ψs × ψr'`, ou seja `ψr' × ψs`. Com o
  sinal trocado o controlo empurrava para o lado errado.
- No `eletrico.h`, `el_wheatstone` numerava os braços de uma maneira e `el_detector` de outra —
  **duas funções minhas que tinham de concordar e não concordavam.**

**Quando há uma fonte, copiar o sinal da fonte e não o que parece natural.** Quando há duas
funções que se referem ao mesmo objeto, escrever a convenção num comentário e usá-la nas duas.

## 2b. E uma armadilha concreta, que já custou duas compilações

**`I` é a unidade imaginária de `<complex.h>`**, e o `eletrico.h` deste projeto inclui-o. Usar
`I` como nome de variável (a corrente, a amplitude — o nome óbvio em eletrónica) dá um erro de
sintaxe incompreensível: `expected identifier or '(' before '__extension__'`. Aconteceu no
`motor.c` e outra vez no `solar.c`. **Usar `Ic`, `Im`, `corr` — nunca `I`.**

## 3. Só então: a lógica.

Nas duas vezes a lógica estava certa desde o início.

## E repeti-o na rodada seguinte a tê-lo escrito

Na simulação a seguir (`dtcn.c`, o DTC hipercomplexo) pedi banda de `0,05` com um passo de
correção que movia o torque `0,03` por iteração — o ciclo limite ficava em `0,0645`, fora da
banda, e a asserção caiu. **Exatamente o defeito 1 desta nota, uma rodada depois de eu a
escrever.**

Escrever o memory não impede o erro; **consultá-lo antes de escolher os parâmetros é que impede**.
A conta que faltou é de cinco segundos: *quanto move cada passo × quanto tolera a banda?*

## E o antídoto que funcionou das três vezes

**Medir a LEI, não escolher os números.** No `dtcn.c` a correção não foi apertar o passo até a
asserção passar: foi medir que o ripple **encolhe com o passo** (`76×`, de `0,0145` a `0,0002`).
E aí o que era um erro de parametrização virou um resultado — porque *o ciclo limite não é defeito
da simulação, é o ripple de torque do DTC*, e é exatamente por ele que o inversor multinível
existe. Ver [[feedback-assercoes-vazias]], secção do número escrito de cabeça.

Cuidado com o gémeo: ao medir a lei, **não exigir monotonia estrita**. Depois de convergir, o
valor oscila no ruído numérico, e uma asserção ponto-a-ponto passaria a medir o ruído. Medir do
primeiro ao último.

## E o que apanhou, das duas vezes

O **par de caminhos**. No motor foi `cruzado(ψ,ψr)` contra `|ψ||ψr|·sen(γ)` — a discordância
denunciou o sinal. Na ponte foi a asserção de que o detector lê zero no equilíbrio. Ver
[[feedback-dois-caminhos]]: uma asserção sozinha teria confirmado o meu erro; duas contas que
*têm* de fechar não precisam que eu saiba a resposta.

Ver também [[project-checkpoint-2026-08-01-maquinas]].

## 01/08/2026 (noite): falhei os DOIS primeiros itens na MESMA peça

No `dominios.c`, e sem consultar isto antes:

1. **O SINAL** — escrevi `/(-12h)` na derivada de cinco pontos, onde é `/(12h)`. O termo `B·y'`
   entrava invertido e o resíduo dava `1,3e-1` onde devia dar `1e-8`. Corrigido o sinal: `7e-9`,
   **sete ordens de grandeza**.
2. **A ESCALA** — reutilizei `h=0,02` do `tikz.c`, onde `C=1`. Com `C=250` o Euler **diverge**, e o
   pdflatex morreu com *"Dimension too large"* nas cinco figuras. O passo tinha de sair de `w`
   (`h·w = 0,05`); com isso as cinco compilam.

**Os dois estavam escritos aqui, por ordem, e mesmo assim fui direto à lógica.** Esta nota já dizia
*"escrever não impede, CONSULTAR antes de escolher os parâmetros é que impede"* — e a prova é que
falhei outra vez. **O gatilho concreto a treinar: ao COPIAR um parâmetro numérico de outro medidor,
parar e verificar contra que escala ele foi escolhido lá.** O `h=0,02` era certo no `tikz.c` e
errado aqui, e nada no símbolo `h` avisa disso.
