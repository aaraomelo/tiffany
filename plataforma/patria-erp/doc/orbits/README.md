# Cosmologia da delegação (órbitas × mecânica celeste)

Experimento que constrói um **grafo de delegação grande com ciclos** e verifica
empiricamente as "órbitas" do modelo de `../casl-propagation.tex`, à luz da
mecânica celeste de `hiper/circular/paper_metrica_quantica_algebras.tex`.

```
python3 delegation_orbits.py   # gera delegation_orbits.png + verificações
```

## A ponte formal

| Mecânica celeste (paper) | Delegação (casl-propagation) |
|---|---|
| Espaço de álgebras = de Sitter 2D `ds²=−cos²ψ dθ²+dψ²` | grafo de delegação (dirigido finito, **com ciclos**) |
| Energia de Noether `E=cos²ψ·θ̇` **conservada** (vetor de Killing θ) | intensidade `δ` **dissipada** (−1 por salto) |
| Geodésica → **órbita fechada** (Euclidiana S²: ψ oscila ±arccos L) | trajetória → **espiral** que decai a δ=0 |
| — | a **atenuação é o atrito** que falta ao caso conservativo |

A tese (Lema da energia bem-fundada, §4 do artigo): como `δ` decresce
*estritamente* a cada aresta válida, ela é um **variante bem-fundado**. Onde a
órbita de de Sitter se fecha (energia conservada), a órbita de delegação
**espirala para dentro** até `δ=0`. Por isso **ciclos no grafo são inofensivos**
— não é preciso aciclicidade (DAG); a segurança vem da atenuação.

`δ=∞` corresponde ao caso sem atrito (energia conservada, órbita que nunca
decai) — reservado às autoridades intrínsecas (operador, dono do tenant).

## O que é verificado (num grafo de 3000 nós, 12012 arestas, 2880 em ciclos)

- **V1 — terminação apesar de ciclos**: a maior cadeia de re-delegação
  realizada é `≤ δ0`. Nenhuma órbita é infinita.
- **V2 — `δ` é energia bem-fundada**: toda transição decrementa `δ` em exatamente 1
  (Lyapunov estrito).
- **V3 — ponto fixo `μP`**: o alcance `|Pᵏ(raiz)|` satura em `k≤δ0` — o menor
  ponto fixo (Kleene de baixo), o fecho transitivo da delegação válida.
- **V4 — monotonicidade**: o alcance é não-decrescente em `k`.

## Figura `delegation_orbits.png`

- **(a)** A órbita (ciclo do grafo): espiral dissipativa (delegação) vs círculo
  de raio constante (de Sitter conservativo). Raio = energia `δ`.
- **(b)** Token orbitando o ciclo: revisita os mesmos nós (a órbita), mas `δ` só
  decresce.
- **(c)** Alcance `|Pᵏ|` → ponto fixo `μP`.
- **(d)** Geodésica conservativa (seção euclidiana S² do paper): órbita fechada,
  energia conservada — o contraponto sem atrito.

> Não é uma identificação física: é uma **analogia dinâmica** rigorosa. O caso
> conservativo (de Sitter/esfera) e o dissipativo (delegação) diferem
> exatamente pelo termo de atrito = atenuação. É isso que torna o sistema
> seguro a escala, com ciclos e tudo.

## de Sitter 4D — nível de campo (`desitter4d.py`)

```
python3 desitter4d.py
```
Verifica (Christoffel→Riemann→Ricci, numérico em 12 pontos) que a forma estática
`ds²=−cos²ρ dt²+dρ²+sin²ρ dΩ²₂` é Einstein `R_uv=3 g_uv`, `R=12` → **dS₄
confirmado**. O `n=2` reproduz o dS₂ do paper (controle). Geral: `k` dims de
campo → dS_{2+k}.

## Cosmos de autorização — a leitura operacional (`authorization_cosmos.py`)

```
python3 authorization_cosmos.py   # gera authorization_cosmos.png
```
Protótipo da **Reading B**: o estado de autorização como universo observável.
Detecta anomalia pela **geometria**, sem inspecionar o conteúdo das regras:
massa autorizativa `M(p)=Σδ` (estrelas vs nós escuros), órbitas (ciclos =
sinal organizacional), curvatura `C(p)=emitida/recebida` (hubs/gargalos),
horizonte `δ=0` (verde/amarelo/vermelho), e **sinais vitais geométricos**
(baseline vs anomalia). No exemplo, um surto leva `nós em órbita 3→45` e
`delegações 66→143` — visíveis instantaneamente no contraste (a) calmo × (b)
perturbado, sem ler uma regra sequer.
