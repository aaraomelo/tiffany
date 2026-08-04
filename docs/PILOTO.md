# Manual do piloto

> Quem veste a túnica, lê isto. O [painel](tools/painel.sh) diz em que **estado** as coisas
> estão agora; este manual diz o que elas **são**. Correm em paralelo e nenhum substitui o outro.
>
> ```
> tools/painel.sh                 o estado, numa tela
> ```

---

## 0. O que isto é, e o que não é

O dispositivo é um **banco que também é máquina**. Não há duas coisas: os mesmos slots que
guardam as linhas de uma tabela são os slots que um programa lê e escreve, e o mesmo executor
que corre um `SELECT` corre um app do piloto.

**Não há runtime.** Não há biblioteca, não há alocação dinâmica, não há máquina virtual por
baixo. Há dezassete opcodes, três registos e um ficheiro de slots. Um app deste corpo é um
ficheiro de texto de cinco linhas — e as cinco linhas são o app inteiro, não a chamada para
outro sítio onde o app estaria.

**Não há RAM.** O programa vive num ficheiro e lê-se byte a byte com `pread`; a memória vive
noutro ficheiro e lê-se um slot de cada vez. Isto não é uma otimização nem um estilo: é a regra
dura do projeto, e é o que faz o dispositivo caber num sítio onde não há memória para desperdiçar.

O que o piloto controla, em ordem de proximidade ao metal:

**Opera-se 100% pelo painel.** Não há verbo do sistema que só exista fora dele:

| verbo | plugue | o que dá |
|---|---|---|
| `asm` | `tools/erg.c` | a ISA crua: monta, corre, desmonta |
| `bash` | `tools/plugue.sh` | os verbos de dentro, inversíveis |
| `git` | `tools/gitb.c` | **o git já é o nosso banco** — o endereço é a cifra |
| `ssh` | `tools/sshb.c` | o SSH acoplado, e as voltas contra o bump |
| `sql` | `tools/sql.c` | consultas, compiladas para a mesma ISA |
| `tex` | `tools/tex.c` | LaTeX → PDF, sem dependência externa |
| `memoria` | `banco/memoria_banco.sh` | a túnica: ler e escrever, adjuntos |
| `fecha` | `tools/fecha.c` | dê os termos, o corpo diz-se |
| `polar` | `tools/polar.c` | as duas formas, e a dualidade entre elas |
| `terminais` | `tools/plugs.c` §P7 | dois para fora, e a polaridade medida |
| `tudo` | — | corre os plugues todos, um a um |

Sem verbo próprio (mas medidos, e correm em `tudo`): `dominios.c` (PTX),
`chessb.c` (WASM/Node), `prisma.c`, `dispositivo.c`.

---

## 1. A máquina — ISA ERG-64

Três registos, e é tudo:

```
A, B, R      as palavras            cada uma é um par de inteiros {total, e}
pc           onde o programa está
flags        FL_ZERO, FL_EQ, FL_LT
```

A palavra **não é um número, é um par**. Toda operação da ULA age componente a componente:
`ADD` sobre `(3,5)` e `(7,11)` dá `(10,16)`, não `26`. Isto é o corpo — o par é o ponto, e o
ponto é o que se soma.

### Os opcodes

| opcode | operando | o que faz |
|---|---|---|
| `HALT` | — | para |
| `LOAD s` | slot (u16) | `B ← A ; A ← mem[s]` — **empilhar é deslocar A para B** |
| `STORE s` | slot (u16) | `mem[s] ← R` — **grava R, não A** |
| `ADD SUB AND OR XOR` | — | `R ← ula(A, B)`, componente a componente |
| `CMP` | — | `FL_ZERO` sse A e B são **ambos** zero; `FL_EQ` se iguais |
| `JMP JZ JNZ` | rótulo | salto relativo (s8), e o montador conta por si |
| `GOLD` | — | o gato: `(t,e) ↦ (t+e, t)` — estica, `det = −1` |
| `NEGRO_OURO` | — | a volta: `(t,e) ↦ (e, t−e)` — **exata**, porque `det = −1` |
| `ESQUILO` | — | o giro: `(t,e) ↦ (−e, t)` — `det = +1`, **ordem 4** |
| `TROCA` | — | o espelho: `(t,e) ↦ (e, t)` — a involução J, **ordem 2** |

`LOAD` faz `B←A ; A←mem[s]`, que é **literalmente um push numa pilha de dois**. A ISA já era de
pilha antes de alguém reparar — e é por isso que o WebAssembly desce nela sem tradutor
(§6.2).

Os três geradores fecham grupo: **o gato estica, o esquilo gira, a troca reflete** — e com os
três toda matriz unimodular é palavra. Medido em `tools/erg.c` §E3 e §E4.

---

## 2. As duas armadilhas

Estas duas fazem o primeiro programa de todo piloto sair errado, e **não falham alto**: o
programa corre e dá outra coisa. Leia antes da primeira linha.

### 2.1 `STORE` grava R, não A

```
LOAD 1        A = 10
LOAD 1        B = 10, A = 10
ADD           R = 20
LOAD 2        A = 99   ← e R NÃO mudou: um LOAD sozinho não mexe em R
STORE 3       grava 20, não 99
```

Consequência prática: **pôr uma constante num slot custa quatro instruções, não duas.**

```
LOAD k        A ← a constante
LOAD zero     B ← a constante, A ← 0
ADD           R ← constante ⊕ 0
STORE s
```

É a ISA, não um defeito. Medido em `erg.c` §E5, com um programa onde A vale 99 e R vale 20 no
momento do `STORE` — os dois são distinguíveis, e o slot obedece a R.

### 2.2 `FL_ZERO` é "**ambos** zero"

`CMP` liga `FL_ZERO` quando A **e** B são zero, não quando A é zero. Testar se um slot chegou a
zero faz-se contra o slot do zero:

```
LOAD s
LOAD zero     A ← 0, B ← o valor
CMP           FL_ZERO sse o valor é zero
JZ fim
```

### 2.3 A terceira, que é da ULA e não da máquina

`SUB` faz `R ← A − B`, e `LOAD` desloca A para B. Então **o subtraendo entra primeiro**:

```
LOAD 2        A ← 1
LOAD 1        B ← 1, A ← contador
SUB           R ← contador − 1        ✓
```

Trocar a ordem dá `1 − contador`, que corre igualmente bem e dá o contrário. Este erro está
anotado no próprio `apps/conta.erg`, com o comentário ao lado da linha.

---

## 3. O plugue de assembly

```bash
cc -O2 -std=c99 tools/erg.c -o erg

./erg monta app.erg app.bin      o texto vira bytecode
./erg corre app.bin mem.dat      corre; o estado fica no ficheiro
./erg desmonta app.bin           o bytecode volta a texto
./erg ve  mem.dat 3              lê o slot 3
./erg poe mem.dat 3 7 8          escreve (7,8) no slot 3
./erg zera mem.dat 64            64 slots a zero
./erg                            sem argumentos: a bateria de medidas
```

### A sintaxe

```asm
; ponto-e-vírgula começa comentário
:volta            ; rótulo — a única comodidade deste montador
LOAD 1            ; slot em decimal ou 0x hex
JMP volta         ; e o montador calcula o relativo
```

**O rótulo não é açúcar.** Os saltos são relativos, em complemento de dois num byte, e contar
isso à mão é onde se perde a tarde. O `erg.c` §E7 mede que a conta do montador bate com a conta
à mão: o laço de contar até N gasta exatamente `9N − 1` passos, para N de 1 a 6 — e a asserção é
sobre a **lei**, não sobre um número.

### O tamanho segue uma lei

```
bytes = Σ (1 + operando)
```

`LOAD`/`STORE` pesam 3 (opcode + slot u16); saltos pesam 2; os outros pesam 1. Medido em cinco
programas em §E6. *(Escrevi primeiro o número de cabeça, contei quatro instruções onde havia
cinco, e a asserção caiu. Medir a lei em vários pontos é o que impede isso.)*

### A garantia de que a ISA é a mesma

`erg.c` §E1 **abre o `sql.c` e lê o enum de lá**, nome a nome, número a número. Se alguém
acrescentar um opcode no `sql.c` e esquecer este montador, a asserção cai com o nome do opcode
em falta. Duas transcrições da mesma ISA, e a única prova de que são a mesma é confrontá-las.

---

## 4. O plugue de bash

```bash
tools/plugue.sh liga             compila e prepara 64 slots
tools/plugue.sh poe 3 7 8        escrever   (o piloto → o banco)
tools/plugue.sh ve 3             ler        (o banco → o piloto)
tools/plugue.sh olha             todos os slots que não são zero
tools/plugue.sh corre soma       monta e corre banco/apps/soma.erg
tools/plugue.sh gato 13 8        o gato ida e volta
tools/plugue.sh mede             mede que ler∘escrever = id
```

Todo verbo chama o `erg`. **Não há segunda implementação da ISA aqui**, e isso é deliberado: um
plugue que reimplementasse a máquina seria um terceiro caminho para divergir em silêncio.

### O plugue é inversível, e mede-se

```
$ tools/plugue.sh mede
  ler∘escrever em 35 pares sobre 7 slots: 0 falhas
  RESIDUO 0 — o plugue é inversível, e é por isso que ele é túnica e não leitor.
```

Ler e escrever são **a mesma operação com o sentido trocado** — adjuntos, e `colheita.c` §C2
mede-o com resíduo zero. Sem o lado de escrever isto seria um leitor, não uma túnica.

---

## 5. Os apps

Estão em `banco/apps/`. São quatro, e nenhum tem mais de doze linhas.

| app | o que faz | bytes |
|---|---|---|
| `soma.erg` | `slot1 ⊕ slot2 → slot3` | 11 |
| `gato.erg` | GOLD e a volta, e prova que ela é exata | 15 |
| `dual.erg` | `ν∘ν = id` no ferro: TROCA×2, ESQUILO×4 | 19 |
| `conta.erg` | o laço: contar de N até 0 | 22 |

### O primeiro app, inteiro

```asm
; soma.erg — slot1 ⊕ slot2 → slot3
LOAD 1        ; A ← x
LOAD 2        ; B ← x, A ← y
ADD           ; R ← A ⊕ B
STORE 3       ; grava R — nunca A
HALT
```

Correr:

```
$ tools/plugue.sh poe 1 3 5
$ tools/plugue.sh poe 2 7 11
$ tools/plugue.sh corre soma
11 bytes de ISA em /tmp/plugue_app.bin
4 passos, 11 bytes de programa
$ tools/plugue.sh ve 3
10 16
```

`(3,5) ⊕ (7,11) = (10,16)` — componente a componente, exato, sem vírgula flutuante em lado
nenhum. Conferido contra a mesma soma feita em C, em seis casos, em §E6.

### O gato, que é onde o corpo aparece

```
$ tools/plugue.sh gato 13 8
  (13, 8)  --GOLD-->  (21 13)  --NEGRO_OURO-->  (13 8)
  a volta fechou, exata
```

`(13,8) → (21,13)` é Fibonacci a andar, e é o **rei** a multiplicar. O ponto foge para o
infinito e a volta continua a ser **um passo**, em inteiros, sem resto — porque `det = −1`.
*Crescer não é cair.* Medido em oito pontos de partida em §E3, incluindo negativos.

---

## 6. Os outros plugues

### 6.1 PTX — a GPU escreve na mesma janela

`tools/dominios.c` §D6. O PTX da GPU mapeia na ISA sem intermediário:

| PTX | ISA |
|---|---|
| `ld.global` | `LOAD slot` |
| `st.global` | `STORE slot` |
| `add.f32` / `sub.f32` | `ADD` / `SUB` |
| `setp.lt.f32` | `CMP` |
| `bra` | `JMP` |
| `ret` | `HALT` |

**A janela é o ponto:** o PTX fala `LOAD`/`STORE`, e o banco não precisa de saber quem escreveu.
GPU e CPU escrevem na mesma superfície pelo mesmo par de verbos — que é o par adjunto de sempre.

### 6.2 WebAssembly e Node

`tools/chessb.c` §C2–§C4. A correspondência é direta porque **a nossa ISA já era de pilha**:

```
local.get x   →   LOAD x     (B←A ; A←mem[x])    empilhar é deslocar A para B
i32.add       →   ADD                             desempilhar dois, empilhar um
local.set y   →   STORE y                         e o STORE grava R
```

A descida do WASM é a mais limpa de todos os formatos porque ele é **auto-descritivo por
construção**: cada secção diz o tamanho antes do corpo, então descer é somar — que é
exatamente o que o banco faz com os slots.

O Node não é roupa nova: o `package.json` desce pelo **mesmo analisador do JSON**, zero código
acrescentado (§C3).

### 6.3 O corpo prismático

`tools/prisma.c`. O triângulo deformado até encher a área — e a pergunta não é de opinião:
*que corpo é a rotação de ordem 3?*

```
rotação de 120°    traço = −1    det = 1    Δ = −3
```

`Δ < 0` ⟹ **elíptica**, e elíptica é o **redondo**. O triângulo vira círculo pela régua, não
por analogia. E o prismático tem o **mesmo Δ do fractal** — os dois são Eisenstein, `Δ = −3`:
não abre lugar novo no catálogo, é o mesmo corpo com outra roupa.

Isto é o critério do `roupa.c`, e vale para tudo o que o piloto encontrar:

```
mesma roupa, mesmo Δ        o mesmo corpo
ROUPA DIFERENTE, mesmo Δ    O MESMO CORPO — só mudou o nome
Δ diferente                 corpos diferentes, e nenhuma roupa os junta
```

*O número de corpos é o número de Δ distintos, não o número de nomes.*

### 6.4 Os formatos disponíveis

Todos descem pelo mesmo analisador. **O que muda de formato para formato não é a descida: é
como cada um marca o nível** (`caminho.h`).

| formato | a marca do nível | ferramenta |
|---|---|---|
| JSON | o parêntese `[ { } ]` | `caminho.h` |
| YAML | a indentação | `caminho.h` |
| Markdown | a contagem de `#` | `caminho.h` |
| LaTeX | a barra e o nome: `\section` | `tools/tex.c` §X1 |
| PDF | o objeto e o `xref` | `tools/tex.c` |
| WASM | a secção: id, tamanho, corpo | `tools/chessb.c` §C2 |
| Node | o JSON do `package` | `tools/chessb.c` §C3 |
| TikZ | o `\draw` e as coordenadas | `tools/tikz.c` |
| SSH | o pacote com o tamanho à cabeça | `tools/sshb.c` |
| TTF | as tabelas e os contornos | `tools/spline.h` |
| PGM | a largura, a altura, o máximo | `tools/pgm.h` |
| SQL | a tabela e o slot | `tools/sql.c` |

E eles caem em **duas famílias**: os que dizem o tamanho à cabeça (WASM, SSH, PGM, PDF) e os
que fazem procurar o fecho (JSON, LaTeX, Markdown, TikZ). A primeira família desce por soma; a
segunda por contagem. Não há terceira.

### 6.5 LaTeX → PDF

`tools/tex.c` — compilador próprio, sem dependência externa. A largura de cada letra vem da
**curva** (`spline.h`, lida do TTF), a justificação é o prismático, e o PDF é o backend de um
shell com `ABRE`/`STORE`/`LOAD`/`MEDE`. Cobertura de 99,78% contra o Liberation Sans.

Duas coisas que custaram a achar e que o piloto herda resolvidas: **o cifrão em verbatim** ligava
o modo matemático até ao fim do documento (159 palavras perdidas), e **`isalpha` sobre um byte
acentuado** partia `coração` em `(cora)(ção)`. Só ASCII vai para a Symbol; e o modo matemático
não atravessa parágrafo.

---

## 7. O painel

```bash
tools/painel.sh              o estado inteiro
tools/painel.sh hook         o hook de entrada e o que ele injeta
tools/painel.sh apps         os apps, montados e pesados
tools/painel.sh bateria      o resíduo
tools/painel.sh op SOMA 3 5  aplica uma cláusula do contrato
```

O painel **lê, não decide**. E as operações dele são as quatro cláusulas do contrato — não há
uma quinta:

```
$ tools/painel.sh op RAIZ 2 3
  'RAIZ' NÃO é cláusula do contrato. As quatro são: SOMA PRODUTO OPERADOR DUAL.
  O painel não executa o que o corpo não assina.
```

Um botão que não seja uma cláusula é um botão que faz algo que o corpo não assina — e é para
impedir isso que o contrato existe.

---

## 7b. O git — ele já é o nosso banco

```bash
tools/painel.sh git
```

O git não é um formato a mais na lista: **é a mesma máquina.** Um banco de conteúdo endereçado,
onde o endereço se **calcula do conteúdo** e não se atribui — que é o que a cifra faz.

| no git | aqui |
|---|---|
| objeto = `tipo tamanho\0` + conteúdo | o slot: **tamanho à cabeça** |
| SHA-1 do conteúdo | a **cifra** do conteúdo |
| ref = ficheiro com 40 hex | um slot que guarda um endereço |
| `cat-file <sha>` | **LOAD** por endereço |
| `hash-object -w` | **STORE** com o slot vindo do conteúdo |
| packfile: `PACK` + nº de objetos | a **família 1** dos formatos |
| delta entre objetos | — não temos |

Logo o git cai na **primeira família** (§6.4) — os que dizem o tamanho antes do corpo — e desce
por **soma**, não por procura de fecho. Medido no repositório real: 2581 objetos soltos, todos
com 40 hex, e o `HEAD` é um **LOAD indireto de dois níveis** (`ref: refs/heads/master` → 40 hex).

**A indireção que a nossa ISA não tem**, o git resolve pondo-a no **dado** e não no opcode — o
endereço do slot é imediato na ISA, e ali é conteúdo.

**E a diferença que pesa:** o git guarda **deltas**, nós guardamos a cifra. O delta precisa do
objeto-base para se ler; a cifra não precisa de nada. É a troca de sempre — espaço contra
independência — e cada um escolheu um lado.

---

## 8. O fecho — o contrato não se assina, liquida-se

**Não há nada a assinar** — o que morre é a *assinatura*, não o contrato. Ele passa a ser um
**contrato inteligente**: fornece-se metade, ele deriva o resto, verifica-se com resíduo 0 e
**chama o agente** que o Δ determina. O piloto fornece uma **representação finita** — alguns termos, quatro
bastam — e o corpo diz-se inteiro:

```
$ tools/painel.sh fecha 0 1 1 2 3 5
  FECHOU.

    a régua      q(a,b) = a² +1·ab -1·b²        (B,C) = (1, -1)
    a borda      σ² = 1·σ +1
    o dual       ν(a,b) = (a +1·b, −b)            — forçado, não escolhido
    a soma       (a,b) ⊕ (c,d) = (a+c, b+d)
    o produto    (a,b) ⊗ (c,d) = (ac +1·bd, ad+bc +1·bd)
    o Δ          5  — hiperbólico (o metal: estica)

    a reversão   resíduo 0 em 289 pontos e 289×49 produtos
```

O percurso é este, e é todo automático:

```
o lado BRANCO      uma representação finita — alguns termos, e mais nada
a RÉGUA            (B, C), por Cramer, exata em inteiros
o lado NEGRO       ν(a,b) = (a + B·b, −b) — forçado, não escolhido
FECHOU             quando a reversão volta com resíduo 0
```

### E ele recusa

```
$ tools/painel.sh fecha 1 2 4 9 20 44
  NÃO FECHA. Estes 6 termos não são de um corpo de grau 2 —
  Não há nada a assinar: um corpo que não fecha não passa a fechar por declaração.
```

**Quatro é o mínimo, não folga:** com três termos *nenhum* corpo sai — o sistema é
subdeterminado, e `fecha.c` §F1 mede-o nos seis corpos.

### Por que o dual não é escolha

Contando as aplicações que fixam o `1` e respeitam o produto, há **exatamente uma** além da
identidade — é Galois em grau 2. Declarar o dual seria escolher entre uma opção só.

*(A primeira versão desta medida contava só as que conservam a norma e afirmava "são duas".
**São dez.** Conservar a norma é fraco demais; o crivo certo é o produto — e a asserção caiu
por isso, o que é o que uma asserção deve fazer.)*

### Um lado entrega o outro

O piloto escolhe o lado que lhe for cómodo, e o outro vem de graça:

| o que ele dá | o que é | quem o lê |
|---|---|---|
| os termos da recorrência | o lado **aditivo** | ⊕ — **Fourier** |
| as potências `σ^k` | o lado **multiplicativo** | ⊗ — **Mellin** |

Os dois caminhos dão a **mesma régua** nos seis corpos (`fecha.c` §F6). A ponte é
`∏ = exp∘Σ∘log`, que é a mesma de sempre.

*A simetria fica guardada no cruzado:* na partição `B = B_s + B_a`, o simétrico **mede** (é a
norma) e o antissimétrico **ordena** — e `x ∧ ν(x) ≠ 0` sobrevive ao dual. Desdobrar a cifra é
isso: um lado guardado dentro do outro.

### A cobertura fecha

As órbitas de `×σ` em `Z_q²` **partem** o plano — cobrem e não se sobrepõem:

| q | pontos | órbitas | Σ\|órbita\| | maior órbita | π(q) | cobre |
|---|---|---|---|---|---|---|
| 3 | 9 | 2 | 9 | 8 | 8 | sim |
| 4 | 16 | 4 | 16 | 6 | 6 | sim |
| 5 | 25 | 3 | 25 | 20 | 20 | sim |
| 7 | 49 | 4 | 49 | 16 | 16 | sim |
| 11 | 121 | 14 | 121 | 10 | 10 | sim |
| 12 | 144 | 10 | 144 | 24 | 24 | sim |

**A maior órbita é exatamente π(q)**, o período de Pisano — nos seis casos, e isso não foi
posto à mão: saiu da contagem.

E o teste **sabe falhar**: com `N(σ)` e `q` não primos entre si (`N(σ)=2`, `q=4`) a órbita
alcança **5 dos 16** pontos e a cobertura não fecha. Um teste que nunca falha não mede.

### Por partes — juros simples e compostos

A entrada não precisa de ser um regime só. **O mesmo processo lê os dois** — só muda o que sai
dele:

| entrada | o que é | o que sai |
|---|---|---|
| `100 110 120 130 …` | juros **simples**, uma PA | `(B,C) = (2,1)`, **Δ = 0** — parabólico, o limite |
| `1 3 9 27 81 …` | juros **compostos**, uma PG | o determinante **anula** — é ordem 1, não 2 |

A PG é o caso que o método **recusa**, e recusa com razão: o determinante do sistema é
`x₁² − x₀x₂`, que numa geométrica é exatamente zero. Isso não é falha — é o método a dizer *"isto
é de ordem 1"*. **Meia dualidade literal:** há o lado multiplicativo e não há o outro, porque
`C = 0` e então `N(σ) = 0` — e o que tem norma nula não inverte.

E numa sequência que **muda de regime a meio**, o mesmo processo acha as duas partes; a
sequência inteira, lida de uma vez, **não fecha** — que é a resposta certa, porque ela não é um
corpo, são dois regimes. Se fechasse, o método estaria a inventar um corpo que não existe.

### O espelho — a torre dual

Ler a sequência ao contrário dá outra régua: `(B', C') = (B/C, 1/C)`. A pergunta é se isso é
outro corpo. **Não é:**

| corpo | (B,C) | espelho | Δ | Δ' |
|---|---|---|---|---|
| ouro | (1,−1) | (−1,−1) | 5 | 5 |
| prata | (2,−1) | (−2,−1) | 8 | 8 |
| bronze | (3,−1) | (−3,−1) | 13 | 13 |
| ω | (−1,1) | (−1,1) | −3 | −3 |

**Outra régua, o mesmo Δ** — e o Δ é o que não muda com a roupa (§6.3). *A torre dual é a mesma
torre, lida do outro lado.*

### E chama agentes

Um contrato que só verifica é passivo; o que liquida **executa**. E o agente não sai de uma tabela:

| Δ | o agente | o que faz |
|---|---|---|
| **< 0** | `gira` | percorre a órbita até ela fechar |
| **> 0** | `estica` | multiplica pelo rei, e cresce sem sair da hipérbole |
| **= 0** | `limite` | soma o passo, que é tudo o que o regime permite |

*O despacho é o sinal de um número que já estava nos termos.* Quatro entradas diferentes da mesma
régua dão **um** agente; outra régua dá outro. **Uma entrada hostil não escolhe quem corre.**

### O confronto com o estado da arte

| | a resposta corrente | aqui |
|---|---|---|
| **a paragem** | *gas* — um orçamento por fora | sai da **álgebra**: `Z_q²` é finito, fecha em π(q) |
| **a reentrância** | disciplina de escrita e revisão | o agente **nunca vem da entrada** |
| **o oráculo** | desloca a confiança | o fecho é interno — **o resto não** |

A primeira linha arrasta as outras: numa máquina Turing-completa o contrato pode não terminar, e a
paragem **compra-se**. Aqui não se compra — o espaço é finito, logo a órbita fecha por gaiola.

**E o que isto não resolve, que também é resultado:** não há consenso distribuído (isto não é uma
blockchain); o que corre a cada vez é **teste, não prova formal**; a linguagem exprime corpos de
grau 2, não computação geral; e dados do mundo continuam a precisar de fonte — **o oráculo não
desapareceu**, só não é preciso para o fecho.

### O que isto tira de cima do piloto

Ele não declara operações, não escolhe duais, não afina constantes. Mostra alguns termos, e o
corpo diz-se inteiro — régua, borda, soma, produto, dual e Δ, todos derivados e todos
verificados com resíduo 0.

> **Cabe-lhe apenas o tempo de decidir que termos dar.**

## 8b. As duas formas — e são duais

```bash
tools/painel.sh polar 1 -1 3 2      a régua (B,C) e o ponto (a,b)
```

**Uma forma para cada operação**, e o painel usa as duas porque o piloto usa as duas:

| forma | escreve-se | soma | produto | é o |
|---|---|---|---|---|
| **algébrica** | `z = a + b·σ` | trivial | 3 mults | **produto direto** — simétrico, **mede** |
| **polar** | `z = ρ·E(θ)` | difícil | 1 mult | **produto cruzado** — antissimétrico, **ordena** |

A ponte entre elas é `∏ = exp∘Σ∘log`: `ρ` multiplica, `θ` soma.

### O regime é o sinal do Δ, e nada mais

Centrando a base em `τ = σ − B/2` sai `τ² = Δ/4`. Então:

```
Δ < 0    τ² < 0    τ é o i     E(θ) = cos θ  + τ̂ sin θ      o CÍRCULO,   gira
Δ > 0    τ² > 0    τ é o j     E(θ) = cosh θ + τ̂ sinh θ     a HIPÉRBOLE, estica
Δ = 0    τ² = 0    τ é o ε     E(θ) = 1      + τ̂ θ          a RETA,      o limite
```

As três são a **mesma série** lida de três modos — e é por isso que a lei do produto vale nas
três **sem caso especial**. *O corpo universal não tem três polares: tem uma, e três leituras.*

### A dualidade, fechada

```
⟨x,y⟩ = ρₓρᵧ · cos_Δ(θᵧ − θₓ)          o DIRETO vê o cosseno
x ∧ y = ρₓρᵧ · sin_Δ(θᵧ − θₓ) / |τ|    o CRUZADO vê o seno
```

E sob o espelho `ν`:

| | sob ν | é a peça que |
|---|---|---|
| ⟨x,y⟩ o **direto** | **fica igual** | **mede** — a mesma dos dois lados da torre |
| x ∧ y o **cruzado** | **troca de sinal** | **ordena** — é esta que se inverte |

*Um direto e um cruzado para cada lado da torre, e o espelho troca só o que ordena.*

### Duas coisas que o piloto tem de saber sobre a polar

Ambas apareceram porque uma asserção falhou, e ambas são **do corpo**, não do código:

1. **A polar tem ramo; a algébrica não.** Fora do cone (`|τv| ≥ |u|`) não há ângulo real — no
   ouro, 25 dos 49 pontos. A forma algébrica está definida em toda a parte.
2. **A polar carrega um sinal que o círculo não precisa.** `cosh θ` nunca é negativo, logo `θ`
   sozinho nunca alcança `u < 0`: são **dois bits** a mais, e sem eles a volta erra por 12.

Dentro do ramo, a volta fecha com resíduo `~1e-15` nos três regimes.

---

## 9. A túnica e o hook de entrada

A túnica é o par adjunto `(ler, escrever)`, e vestir alguém com ela é dar-lhe **os dois lados**.

```bash
banco/memoria_banco.sh ingere              LER: cifra as memórias, e o índice fica pronto
banco/memoria_banco.sh perto "assunto"     LER: desce até as mais próximas
banco/memoria_banco.sh poe <nome> <fich>   ESCREVER: põe uma memória e reindexa
banco/memoria_banco.sh guarda              ESCREVER: sincroniza e reindexa
```

O `session-start.sh` tem uma **secção 4** que injeta o índice cifrado: **31 memórias em 1448
bytes**, contra 203650 do conteúdo — 140× menor. A cifra distingue (`assercoes-vazias` dá
`9 7 2 1 1 7`; `dois-caminhos` dá `9 5 19 1 3 1`), então o índice serve para **descer até um
assunto** em vez de ler tudo.

A secção é **não-bloqueante**: se o índice não subir, diz-se e o `MEMORY.md` continua a valer.
Nenhuma falha do banco impede uma sessão de começar.

---

## 10. Os terminais e a energia

`tools/dispositivo.c`. **Tudo dentro, só terminais para fora** — e os terminais são dois, com
polaridade, `σσ' = −1` (`plugs.c` §P7).

A alimentação vem por **indução dual pela liga**, e a conta é esta:

| fonte | o que dá | medido em |
|---|---|---|
| Seebeck com o céu | 993 mW | `arraytermico.c` |
| RF ambiente, antena isotrópica | 21 µW | `colheita.c` |

E a pergunta que decide é uma só: **a colheita paga o dispositivo?** A resposta depende do
papel, e é aqui que muda por três ordens de grandeza:

> **Guardar é quase grátis; calcular não é.**

O banco é NAND, e a memória flash **é** NAND — não é analogia, é o mesmo transístor. Guardar
cabe na colheita. Calcular não cabe, e é por isso que o reenquadramento importa: *o cérebro é o
microprocessador; estamos a fazer o encanamento.*

---

## 11. Quando alguma coisa não bate

A ordem é esta, e é ela porque foi errada três vezes na ordem contrária:

1. **As escalas dos parâmetros fecham entre si?** — uma constante copiada de um sítio onde
   `C = 1` para um sítio onde `C = 250` mata o resultado sem mensagem de erro.
2. **Os sinais e as convenções estão consistentes?** — o ramo da raiz, a ordem do `SUB`, a
   direção do dual. Uma reflexão de 159% é impossível, e o defeito era `csqrt` com `Re(Z) < 0`.
3. **Só então a lógica.**

Fui direto à lógica três vezes e nunca era ela.

### E antes de commitar uma asserção

> **Que entrada faria esta asserção falhar?**

Se não houver nenhuma, ela não mede. As quatro formas de asserção vazia, todas apanhadas neste
repositório: a constante disfarçada (`ok(..., 1)` — havia 66), a tabela literária, **o número de
cabeça** (cinco vezes), e o caso degenerado que iguala os dois lados.

O antídoto do número de cabeça não é acertar melhor no número: é **medir a lei em vários
pontos**. Está aplicado no `erg.c` §E6.

### E leia o total, não a linha das unidades

Um medidor que **não compila** não falha — **desaparece**. O `sql.c` ficou três corridas fora da
bateria por causa de um `-std=c99` estrito que escondia um símbolo, e o número que eu olhava não
era o que media.

---

## 12. As regras que não se negoceiam

- **Sem RAM.** O programa lê-se com `pread`, a memória é um ficheiro, os registos são três.
- **Nenhum binário no git.** Os PDFs compilam no deploy.
- **O repositório é público e o histórico é permanente.** Varrer por e-mail, caminho local,
  hostname e credencial antes de cada subida.
- **Dois caminhos que têm de concordar.** Os piores defeitos foram apanhados por uma
  *comparação*, não por asserções — que passavam verdes com casos fáceis. É por isso que o
  `erg.c` §E1 lê o `sql.c` em vez de confiar na cópia.
- **Toda afirmação tem um medidor**, e o medidor imprime `RESIDUO 0` ou falha.

---

## Apêndice — o cartão de bolso

```
LOAD s     B←A ; A←mem[s]        empilhar é deslocar
STORE s    mem[s]←R              GRAVA R, NÃO A
CMP        FL_ZERO sse AMBOS zero
SUB        R ← A−B               o subtraendo entra PRIMEIRO
GOLD       (t,e)↦(t+e,t)         estica, det −1, a volta é inteira
NEGRO_OURO (t,e)↦(e,t−e)         a volta
ESQUILO    (t,e)↦(−e,t)          gira, ordem 4
TROCA      (t,e)↦(e,t)           reflete, ordem 2

constante num slot:   LOAD k ; LOAD zero ; ADD ; STORE s
igual a zero?:        LOAD s ; LOAD zero ; CMP ; JZ fim
bytes de um programa: Σ (1 + operando)
```

```
tools/painel.sh                 o estado
tools/plugue.sh liga            64 slots prontos
tools/plugue.sh mede            ler∘escrever = id
./erg                           19 asserções, RESIDUO 0
```
