---
name: project-hopfield-torres
description: "Hopfield e a árvore (tools/hopfield.c): a árvore é a Hopfield com a interferência resolvida pela hierarquia — e a correção do Aarão, que era metade da teoria: a torre negra roda com a ordem do i"
metadata:
  type: project
---

# Hopfield e as duas torres — `tools/hopfield.c`

01/08/2026, dois commits (`299691c`, `3b7010b`). **32 unidades, resíduo 0.**

## O que o Aarão pediu, e onde eu parei a meio

> *"explora as redes de Hopfield, é a mesma coisa — a interação é a cifra, dobra e desdobra
> navegando, o presente são as folhas, a árvore já é a hierarquia de memória."*

A parte dele **está certa e está medida**: recuperar é **descer** nos dois, e a **sobreposição não
se parece com o prefixo comum — é ele**, na outra coordenada (4096 pares, resíduo zero).

Mas *"é a mesma coisa"* apagaria o que ele construiu. **A Hopfield satura** (α medido 0,094 contra
o 0,138 de Amit–Gutfreund–Sompolinsky) porque *os pesos são uma SOMA e as memórias interferem*. O
nome que ficou:

> **A árvore é a Hopfield com a interferência resolvida pela hierarquia** — e o preço é o espaço,
> medido e não escondido (N² fixo contra crescer com o que guarda).

## A CORREÇÃO, e ela era metade da teoria

> *"perai, ainda tem só metade. A árvore é uma TORRE, a branca. A parte reversível é a torre
> NEGRA. Então tem ciclos sim, mas ANTISSIMÉTRICOS. Confronta com o R^n."*

Ele tinha razão, e o meu §F1 mostrava onde eu parara: medi *"a energia nunca sobe"* e chamei-lhe o
resultado. **Isso só vale porque Hebb é SIMÉTRICA, e uma simétrica só sabe descer.**

A peça já estava na teoria: **B = B_s + B_a**, a partição única. Medida na rede, dinâmica
**síncrona**, 40 arranques cada e sem um só período diferente:

    B_s simétrica       período 2   ESPELHA   o J, o TROCA (det −1)
    B_a antissimétrica  período 4   RODA      o i, o ESQUILO (det +1), F⁴ = id

**2 e 4 são as ORDENS das duas peças.** Eu tinha escrito *"ponto fixo"* e *"período 2"* — as duas
falsas, e a medida deu melhor do que eu imaginara. É a **mesma** partição do `R^n` (interno mede,
cruzado ordena): não é analogia entre rede e álgebra, é a **mesma decomposição**, e ela é única.

## "Não há o que procurar, só dobra"

A peça que testa isto é **Hadamard, porque ela É a dobra**: `H_2n = [[H_n,H_n],[H_n,−H_n]]`. Sete
dobras de 1 a 128; **8128 pares com produto interno exatamente zero**; as linhas gravadas **são**
pontos fixos e a recuperação fecha em **UMA varredura** — não itera.

**A tabela de unidades** fecha em **GF(2)⁷**: o produto de duas unidades é outra, e o índice é o
**XOR** dos índices (1024 pares, sem exceção). As potências dão as duas interfaces duais, e elas
são as ordens de cima: **branca** `u²=+1` (2, espelha), **negra** `i²=−1` (4, roda).

**O bra-ket:** a matriz de Hebb **é** a soma de `|ξ⟩⟨ξ|`, entrada a entrada e sem resto. O
**estica–contrai** tem forma fechada — `|wξ| = 1−P/N` e `|wu| = P/N`, medidos `0,9375` e `0,0625`,
razão `N/P−1 = 15` exata. *O esticar é RELATIVO, não absoluto: nada passa de 1.*

**A navegação é completamente reversível:** os 256 caminhos voltam exatos, e desce–sobe é uma
involução (ordem 2). *A torre negra existe por isso — sem ela haveria descida e não haveria volta.*

## O meu padrão, nesta peça

**Quatro asserções caíram, todas do mesmo tipo:** *"ponto fixo"* e *"período 2"* (era 2 e 4),
*"norma > 1"* (deu 0,9375 — e o certo era medir contra a **forma fechada** `1−P/N`), e duas sobre
espúrios que não mediam nada (uma falhou, a outra era `esp >= 0`, a constante disfarçada).

**Tirei as duas dos espúrios em vez de as consertar:** *sobre isso não sei o suficiente para
afirmar, então não afirmo.* Ver [[feedback-assercoes-vazias]].

E o achado de método: **quando o número tem forma fechada, medir contra ela vale mil vezes mais que
contra um limiar meu.** `1−P/N` e `P/N` bateram exatos; um `> 1.0` teria falhado sem ensinar nada.
