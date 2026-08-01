---
name: project-checkpoint-2026-08-01
description: "Checkpoint 01/08/2026 — o corpus científico (227 pares, revisto bloco a bloco), a máquina numérica inteira até aos complexos, o 0/0 do Aarão medido, e a teoria ingerida no corpus"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T11:21:38.615Z
---

# Checkpoint 01/08/2026 — tiffany

Estado: **bateria 177 total, 175 verdes, 0 falhas**; `teoria.tex` **78 páginas, 0 pendências**;
292 commits. Tudo empurrado. Continuação de [[project-checkpoint-2026-07-31-noite]].

## O que se construiu

**O corpus científico: 20 → 227 pares**, em blocos de 12–20, cada um seguido de uma revisão
minha à procura de absolutos. E o padrão manteve-se em **onze revisões seguidas**: *nunca
conteúdo errado, sempre a régua que falta*. Duas vezes a régua certa estava escrita **na entrada
ao lado** (monoide/grupo; o traço e os autovalores sobre ℝ).

**A máquina numérica, do parêntese aos complexos** — e o que a fez andar foi nunca haver máquina
nova:

| passo | o que trouxe, e que os anteriores não tinham |
|---|---|
| `+ x ( [ {` | a precedência é a **ordem das dobras**; a roupa não decide, a profundidade decide |
| distributiva | os dois caminhos fecham no mesmo, e há **dual** (fatorar) |
| `- /` | associam à **esquerda**, e a divisão **não fecha em ℤ** |
| `^ raiz` | associa à **direita** — a mesma varredura ao contrário |
| `! mod` | o único **pósfixo**, e o ℤ/n que fecha o círculo com o corpus |
| frações | a máquina passa a **ℚ**, e √2 **continua fora** |
| decimais | **notação**: zero linhas de aritmética |
| percentagem | o `%` com dois sentidos por **posição**, e o dual que não é o simétrico |
| equações | a operação **dual**, feita com o **mesmo avaliador** (3 pontos) |
| complexos | o `i` **já estava** no `zero.c` |

## As três descobertas que valeram mais

**As sementes da cifra são 0/1 e 1/0.** A recorrência das frações contínuas arranca de
`h(-1)/k(-1) = 1/0` e `h(-2)/k(-2) = 0/1`, e a matriz de arranque é a **identidade** — as suas
colunas *são* o infinito e o zero. Não é escolha de coordenadas: sem elas não há cifra. E `J`
troca-as com `J² = −I`. **A dualidade sai de uma divisão por zero**, e o `i` não precisa de ser
introduzido.

**O `i` não está na família dos metais.** `xⁿ − mxⁿ⁻¹ − 1` tem termo constante −1, é toda
hiperbólica e de ordem infinita (o gato); `x² + 1` tem +1, é elíptica de ordem 4 (o esquilo).
*Não há m que os junte.* Por isso o salto para ℂ foi **salto** e não dimensão a mais — e é
exatamente isso que o furo em n=5 separa. **Dizer que a teoria hipercomplexa já continha os
complexos é verdade pela metade: ela contém a família que os cruza, e o cruzamento tem endereço.**

**As duas metades concordaram quatro vezes sem terem sido ligadas** — √2 em ℤ/7, 3×3 = 3+3 em
ℤ/3, 0,1+0,2 exato em ℚ, e a dízima que é da base. O corpus declarou o corpo porque esse é o
critério; o resolvedor declarou o corpo pelo mesmo critério; e depois um passou a **verificar** o
outro.

## Os buracos meus, e o que os apanhou

**O pior:** `sql.c` **não compilava** com o `-std=c99` da bateria, e as suas **87 asserções**
estavam fora da medida há pelo menos três corridas. Passou porque eu lia *"unidades: 7 passaram"*
— sete — e dava por verde; o `1 falhas` do total estava à minha frente em todos os commits.
**O número que eu olhava não era o número que media.**

**O mesmo defeito em três camadas:** de cada vez que a célula ganhou uma componente (denominador,
depois parte imaginária), as cópias da reescrita distributiva **apagavam-na**. Copiar campo a
campo obriga a lembrar de todos os campos, e falhei nas duas. Passou a copiar a `struct` inteira.

**Conta certa por dentro, resposta errada por fora — duas vezes:** `7/2` saía como *"dá 7"* e
`raiz -4` como *"dá 0"*. Na segunda, **o medidor passou verde porque lia a fita e não a resposta.**

**Quatro testes envelheceram**, e isso é bom sinal: `2+3-4` e `2+3^4` deixaram de ser recusa
porque a **linguagem** cresceu; `7/2` e `2^-1` deixaram de parar porque o **corpo** cresceu. *As
afirmações antigas não eram falsas — eram relativas.*

Também: `ct_passo` a devolver `-1` que é *truthy* (8 laços infinitos em potência); a porta
`e_conta` fechada enquanto a máquina já sabia dividir; e uma soma minha errada — `(2x3)x(2x4)` é
**48**, não 144.

## E o instrumento que mais apanhou não é um teste

A linha que compara os **dois caminhos** da distributiva e comenta *"não devia diferir; há defeito
aqui"* encontrou os dois defeitos de cópia. Não é uma asserção — é uma comparação entre dois
caminhos que **têm** de concordar. **Isso apanha o que nenhuma asserção sobre um caminho só
apanharia**, e é o padrão a repetir.

## O corpus ingerido

`tools/corpus.sh` põe as três fontes numa linha: `ciencia.sh`, `semear.sh` e **a teoria, ingerida
pela estrutura que ela própria declara**. 374 pares, 122 vindos dali. *O corpus não pode divergir
da teoria, porque é ela.*

## Aberto

- o `l[8192]` do socket e o `cb1/cb2/ramos` ainda aterram em RAM antes do banco;
- o martelo fatiado (64× em `fatia.c`) ainda não substituiu o `sha256` do `OP_MARTELO`;
- a assistente resolve equações do 1.º grau; o 2.º grau é recusado com o motivo, e fica por fazer.
