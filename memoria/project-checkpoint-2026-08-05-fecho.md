---
name: project-checkpoint-2026-08-05-fecho
description: "Fecho de 05/08 — publicado na Patria, 303 medidores verdes, e três coisas que já existiam e foram RECONHECIDAS"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-06T00:40:04.651Z
---

**Fecho de 05/08 — no ar, com deploy `success` em 3m31s.**

```
bateria  303 : 303 VERDES, 0 falhas        (287 -> 303 hoje)
teoria    58 pp.   catalogo 443 pp.   enredo 360 pp.
RAM      69 153,8 -> 42,5 KB               (informativo; o portao passou a ser o caudal)
caudal   2 041 968 bits/corrida            (-54,7%)
piso     0 bytes — sem CRT nao ha' seccao .bss nem .data
```

## O que define o dia não é nenhum número: é o que já existia

- **O DTC multinível é a máquina sem memória em silício — e é o TCC dele de 2018**, nota 10 dos três membros. Derivámo-la por teorema; a indústria construiu-a porque o modulador custava tempo.
- **O `neuronio.c` de 01/07, 33 linhas**, já tinha a partição dual (`0xAA`/`0x55`), a involução, a leitura sem estado e o par `(medida, ordem)` na saída `[e+o, e]`. **A derivação chegou lá sozinha** — ele tinha-o há meses sem conseguir fundamentá-lo. É o teste mais forte do trabalho, e o único que não depende de acreditar em quem o escreve.
- **O colisor é o relógio**, que já tinha o mesmo teorema provado (`relogio.c §R3`, o pente autodual). Construí-o outra vez com outro nome.

## O que se derivou

- **`(1−s²)·g(p) = 4`** — a norma do evolutivo é o inverso da métrica de Fisher, e o ponto fixo da involução é o mínimo da régua.
- **A velocidade máxima sai do círculo**: `d(p)=min(p,q−p)` cresce só até `q/2`, e o máximo **é** a involução.
- **Pascal é o passo da torre**: `C(n,k)=C(n−1,k−1)+C(n−1,k)` é `A_{n+1}=Aₙ⊕Aₙ†` contado. Isso **desfez uma insinuação** — o `6+3+3+2+1+1` deixou de precisar do nome da física.
- **As quatro equações da expansão**, com só duas independentes: `H²=1/D` (a taxa ao quadrado É o inverso do discriminante), a aceleração, a continuidade e `w`. E a bidualidade dá órbitas de **quatro** em `w`, que **degeneram em dois no ponto fixo**.
- **O ouro branco é o membro dual**: `x²=4x−1`, `det +1` contra `−1` da família, e **sem elementos de norma −1** — confirmado por dois caminhos que não se falam.

## Os revisores: 30 de 37, e os piores eram meus de hoje

- **`dissipa.c` tinha `return n ? 32 : 32`** — os dois ramos iguais. O ficheiro que define a régua do portão **não media**.
- **`κ·D^{3/2}` escrita como conservação exacta** — o medidor desmentiu em 60 de 60. É limite, não identidade.
- **`entrega.c` exigia que o modelo errasse** (`fn >= 2`): quando ele melhorou, o medidor acusou a melhoria como falha. Era a «falha real» que eu reportava há horas.
- **`thm` de π** enunciava `J†=−J` e a prova usava `J²=−1`. **«Um monte é um 4-cubo» era falso** — toda colisão troca a paridade.

## E dois achados que retirei depois de os passar adiante

O revisor do enredo apontou o «eixo com quatro donos» e «25 capítulos contra 18». **Nenhum era defeito:** o primeiro é a *rotação do elenco*, declarada num capítulo próprio (`L5638`); o segundo são **20 escritos + 5 por escrever, um por personagem** — 5 personagens, `2⁵=32`, `16+16`, 8 biduais, `5×5=25`. **É o conforme `(4,1,0)`, o único corpo de grau 5 do catálogo.**

Lição: **passei-os adiante sem verificar se o texto se explicava a si próprio.**

## Ferramentas novas

`tools/dissipa.sh` (o portão, agora é o caudal), `tools/indices.sh` (colisões e o tecto de 383), `tools/involucoes.sh` (27 de 27 lidos, veredito por ficheiro), `lib/relogio.h`, `code/zero.c` (o piso).

**O Ollama saiu inteiro** — 25→10 scripts, 0 chamadas a 11434, e a bateria não perdeu uma unidade.

Ver [[project-checkpoint-2026-08-05-auto-contido]], [[project-checkpoint-2026-08-05-relogio-canonico]], [[feedback-o-controlo-a-tres-linhas]], [[feedback-estrutura-lida-como-ruido]].
