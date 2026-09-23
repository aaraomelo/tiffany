---
name: project-jeve-piloto-e1-e2
description: "JEV no motor wasm: piloto E1+E2 fechado — 350/350, 0 falhas; marginal.wasm (1962 B, proj/marginal); WASM ≡ oráculo ≡ esperado"
metadata: 
  node_type: memory
  type: project
  originSessionId: jev-wasm-piloto
  modified: 2026-09-23T00:00:00.000Z
---

# JEV no motor wasm — piloto E1+E2 (fechado)

## O marco

```text
E1–E2 PILOTO
350/350 unidades
0 falhas
6 casos × 5 sementes LCG (a=1664525, c=1013904223, m=2^32)
n = 1..3
I = 0, 1, 300, 4444
X = 8, 16, 32, 64
WASM ≡ oráculo ≡ esperado
```

Artefacto:

```text
marginal.wasm
1962 B
2 funções: proj, marginal
conecthus/backends/wasm/jev/marginal.c → tools/traduz → assets/figuras/wasm/jev/marginal.wasm
```

## O que o piloto fechou

- **E1 — Conservação no motor**: `G(x)=|{i: π(i)=x}|` com `Σ_x G(x) = |I|` e
  `Σ_y G_q(y) = |I|` por projeção (réplica de `prop:conservacao`/`prop:gq-conserva`).
- **E2 — Marginalização no motor**: marginais da conjunta `D2` == marginais
  diretas `D1` (réplica do `thm:marginal`), e ambas == oráculo JS.
  Cobertura: `n=1..3`, sobreposição (X=8, I=300), campo grande (X=64, I=4444),
  bordas (I=0, I=1).

## Decisão de desenho mantida

`marginal.c` usa **partições por fronteiras na arena** + **odômetro** para ler a
conjunta em ordem — sem `%` nem divisão variável (o `traduz`/`wasm_erg` só aceita
div/rem por 256; ver `claim/runtime.c`). A construção fica integral: o piso é a
contagem, e as normalizações racionais deixam-se para depois (E4, ponto fixo).

## Fronteira epistemológica (mantida)

O piloto valida a **construção do paper sobre o motor**. Não alega nada sobre o
JEV por dentro; `confidence` permanece campo documentado, não reconstruído.

## E4 (fechado depois)

- **Score fracionário sem divisão no motor** — aprovação de desenho do Aarão:
  nada de float, nada de ponto fixo; o objeto primário é o racional.
  `marginal.c` ganhou `escore(X)`: pesos `s_i` na arena,
  \(N=\sum_x s_{q(x)}G(x)=\sum_i s_i G_q(s_i)\) sai inteiro; devolve o par
  exato `(N, |I|)`, e \(N/|I|=E_G[S]\) é **leitura do par** no oráculo.
- Demonstrativos: \(7/3\), \(11/4\), \(5/3\) (G explícito, não LCG);
  borda \(0/5\). Robustez LCG: `X∈{8,16,64}`, `I∈{64,300,4444}`,
  sementes 111/333/777 — pares exatos.
- Resultado: **E1+E2+E3+E4 = 446/446 unidades, 0 falhas**.
- `marginal.wasm` com 3 funções (`proj`, `marginal`, `escore`), 2889 B.

```text
G → G_q → (N, |I|) → N/|I| → E_G[S]
```

## E3 (fechado depois)

- **Composição sobre o mesmo `marginal.wasm`** (nenhuma prova nova):
  Noul/Choice/Score como coordenadas da vista conjunta; cada leitura uma
  marginal da conjunta e \(\sum_y p_q(y)=1\).
- Resultado: **E1+E2+E3 = 422/422 unidades, 0 falhas**, `tests/jev_backends.js`.
- `c^\star` (ponto decisor da Choice) bate com o oráculo em todas as corridas;
  leituras distintas demonstram «uma realização, múltiplas leituras tipadas».
- Marco consolidado no papel em `docs/INTEGRACAO_JEV_WASM.md` (status:
  parcialmente implementado; E5–E6 em aberto).

Próximo: **E5 — Paralelismo**: Noul e Score no mesmo percurso sobre `G` (um
disco, duas roupas); medir que a segunda leitura **não recria** `G` — «avaliadas
em paralelo, cada uma em isolamento».

## E5 (fechado depois)

- **Um disco, duas roupas**: `marginal.c` ganhou `duas(X)` — Noul e Score na
  **mesma varredura** sobre `G` (fronteiras do Noul `c=2` + níveis/pesos do
  Score na mesma arena); Noul marginal, Score por nível, par `(N,D)` e o total
  recontado saem do mesmo percurso.
- Testes por corrida (3 casos × 3 sementes LCG): uma varredura conserva
  `total=|I|`; Noul e Score normalizam; **campo `G` intacto** (a segunda
  leitura não recriou o campo); leitura conjunta == leituras isoladas
  (`marginal` para o Noul; `escore` para `(N,D)`) — «avaliadas em paralelo,
  cada uma em isolamento» cai no selo.
- Resultado: **E1+E2+E3+E4+E5 = 509/509 unidades, 0 falhas**.
- `marginal.wasm` com 4 funções (`proj`, `marginal`, `escore`, `duas`);
  4189 B.
- Cadeia conceptual fechada na prática do motor:
  `G → G_q → (N,D) → N/D → E_G[S] → lim E_{G_n}[S]`.

Próximo: **E6 — demonstração de integração no front** (painel como state;
não é medidor — reutiliza E1–E5; padrão `substratos.js`).

## Bloco E1–E5 (fechado como bloco)

- Progressão didática fechada:
  E1 `G` conserva → E2 marginalização exata → E3 várias leituras sobre um `G`
  → E4 Score→`(N,D)` → E5 leituras paralelas sem recriar `G`.
- O E5 fechou a propriedade estrutural: «uma realização `G` → {Noul, Score}»
  na mesma varredura, campo preservado; **leitura conjunta = leitura isolada**
  por contrato — «uma realização, várias leituras tipadas» materializado,
  sem alegar que o JEV internamente o faz.
- Ponte teórica amarrada: `(N,D) → N/D → lim E_{G_n}[S]`.
- **Decisão**: E6 é o **último selo** — demonstração de integração, não prova
  matemática. A matemática já está validada em E1–E5 (509/509, 0 falhas); o
  navegador só demonstra que a construção acopla ao front. **Nada de novos
  experimentos depois de E6** — não se aumenta contador por aumentar.

## E6 (fechado — o último selo)

- **O painel como state**: a fase de `painel_motor.wasm` (ponto fixo 2^20, wrap
  bit-a-bit, resíduo 0) **é** a realização única — passos `dt` = eventos; bins
  `x_k = X` células. A seção ao vivo `app/src/jev_contratos.js` (padrão
  `substratos.js`, na página do app, após `initSubstratos`): `X=16`; o campo `G`
  acumula a fase lida no topo do quadro (consumidor do caminho livre do
  `relogio.js`; `registra(tick)`); **tabela por pergunta** (Noul/Choice/Score):
  D1 (projecção directa), D2 (marginal da conjunta) e o oráculo — selo
  WASM ≡ oráculo ≡ esperado por linha; o Score mostra o par exacto `(N,|I|)`
  com `E_G[S] = N/|I|`; a Choice o argmax `c⋆`; «o campo G não é recriado»
  (E5 no front); botão «↻ esvazia o campo».
- Correcção de contrato no caminho: `escore` **não** emite níveis por nível —
  o C só devolve `(N, |I|, m)`; os níveis `G_q(s_i)` vêm da coordenada Score da
  leitura conjunta (a D1 já selada). A leitura do front (`leEscore`) e o smoke
  cobrem o contrato real; o C fechado não foi tocado.
- **Smoke da cola** `tests/jev_front.js` — **68/68, 0 falhas**: a mesma
  recursão da fase em ponto fixo, com leituras determinísticas → bins → o mesmo
  `marginal.wasm` (`marginal`/`escore`), sem DOM; casos `ω=0.05 n=300`,
  `0.37 n=500`, `0.9999 n=2000` e o degenerado `1.0 n=1000`. Mede a cola que o
  navegador segura, não a construção (E1–E5 já a selam, 509/509).
- Zero mudança de mecanismo: nenhum C novo, nenhum teorema novo, nenhum
  contador novo — reutiliza o bloco fechado (`/wasm/jev/marginal.wasm` via
  publicDir). `npm run test:jev` (em `app/`) corre os dois medidores.

## Piloto encerrado

- **E1–E6 fechados. A sequência de experiências termina aqui — por decisão, não
  por exaustão**: a matemática validada em E1–E5 (509/509) e o acople ao front
  demonstrado em E6 (68/68 + a seção ao vivo). Não se aumenta contador por
  aumentar.