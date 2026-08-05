---
name: project-checkpoint-2026-08-05-relogio-canonico
description: "Checkpoint 05/08 tarde — RELÓGIO e RÉGUA ficam canónicos, o colisor é sinónimo, e a RAM chega a 54,7 KB"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-05T21:05:47.807Z
---

**Fecho de 05/08 (tarde).** O ciclo da máquina sem memória fechou, e abriu-se outro: a **consolidação dos nomes**.

## A decisão do Aarão, e é estrutural

**Dois grupos de nomes, e só a diferença entre eles interessa:**

| a **estrutura** — uma só, igual para todo corpo | a **individualidade** — números |
|---|---|
| matriz, **RELÓGIO**, viveiro, torre, colisor, cifra do reino | assinatura, grau, métrica, **RÉGUA**, norma, fase, passo, `[σ]` |

**Canónicos: RELÓGIO e RÉGUA.** Os outros só onde nomeiam um *uso* (torre = o relógio a subir de andar; viveiro = duas a combinarem-se; assinatura = quando se contam eixos). A tabela está no **início da teoria**, antes da secção 1, marcada como não provando nada.

E os números da direita **são caminhos específicos na matriz do relógio** — todas as passagens já estavam medidas: `(p,q,r)→n→2ⁿ`; `(1−s²)·g(p)=4`; fase `p/q → [σ]=q`.

## O que se derivou

- **`(1−s²)·g(p) = 4` exacto** — a norma do corpo evolutivo é o inverso da métrica de Fisher, e o ponto fixo da involução é o mínimo da régua. `evolu` aparecia **zero** vezes na teoria.
- **A velocidade máxima sai do círculo**, não de um postulado: `d(p)=min(p,q−p)` cresce só até `q/2`, e o máximo **é** a involução — onde ida e volta coincidem. Já lá estava com `c`: o relógio de luz entre dois espelhos.
- **`3 → 8 → 32 → 16+16`** — a assinatura tem três entradas porque o trial tem três estados; **cada eixo é uma partícula trial e cada partícula trial é uma realização do relógio**. O corpo não tem um relógio dentro: **é `n` relógios**.
- **32 = arestas do 4-cubo** (fecha, `n` par) **e estados do 5-cubo** (desdobra, `n` ímpar) — e `conforme = geométrico + 1 eixo` é a passagem entre os dois regimes.

## Os meus defeitos deste ciclo

1. **Construí o colisor quando o relógio já era isso** — e o objecto tinha **o mesmo teorema já provado** (`relogio.c §R3`, o pente autodual). Agravante novo do [[feedback-a-base-ja-existe]].
2. **[[feedback-estrutura-lida-como-ruido]]** — li meia órbita como ruído de medida.
3. **`git stash pop` falhou e eu comparei o antigo com o antigo** — «IDENTICO» sobre nada. Comparar o que não é o resultado, 4.ª vez.
4. **`git add -A a b c` aborta tudo** se um caminho não existir — o commit levou só a ferramenta, sem o código.
5. **O catálogo não compilava desde `adfbd0e`** — deixei um `\end{tabular}` órfão ao apagar o transplante. Um documento que não compila **não falha, desaparece**.
6. **O índice do disco tem tecto: 383** (47 bits de espaço de utilizador). Estavam 197 usados de 384 e ninguém o dizia.

## Estado

```
RAM      69 153,8 -> 54,7 KB      (-99,92%)
bateria  287 -> 298 : 296 verdes, 2 falhas REAIS (entrega, tresp)
teoria   53 -> 57 pp.    catalogo 431 -> 439 pp.
```

**Ferramentas novas:** `tools/indices.sh` (colisões de índice + tecto), `tools/involucoes.sh` (inventário das involuções, **com veredito e razão por ficheiro** — 15 lidos, 12 por ler), `lib/relogio.h`.

**Aberto:** os 12 por ler; o `tresp.sh` por corrigir a 4 passos; o saneamento do corpo do texto (71 ocorrências de torre/matriz, caso a caso); os 3 colhedores parados.
