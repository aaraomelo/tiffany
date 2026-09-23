# Integração JEV-WASM — proposta de integração e experimentos

Status: **fechado — E1–E6** (o piloto JEV-WASM completo).
E1–E5 fechados no motor
(`conecthus/backends/wasm/jev/marginal.c` → `assets/figuras/wasm/jev/marginal.wasm`,
medidor `tests/jev_backends.js` — **509/509, 0 falhas**). E6 fechado como
demonstração de integração no front (`app/src/jev_contratos.js`, seção ao vivo
no app; smoke da cola `tests/jev_front.js` — **68/68, 0 falhas**).
**Nada se valida depois — o piloto está encerrado.**

Refere o paper `redes/jev.tex` («JEV e Campos de Contagem», fechado
matematicamente) e o motor Wasm da Tiffany
(`assets/figuras/wasm/*.wasm`, `app/src/*`).

## Fronteira epistemológica (mantida do paper)

```text
documentação JEV ≠ mecanismo interno JEV ≠ construção Tiffany
```

Os experimentos **validam a construção do paper sobre o motor** — nunca uma
alegação sobre como o JEV funciona por dentro. Em particular:

- a caixa preta permanece: `state → JEV → distribuição calibrada`;
- `confidence` é um campo documentado no contrato, **não** reconstruído;
- o teorema de marginalização é matemática da própria construção (`thm:marginal`
  em `jev.tex`), portanto independe de qualquer interpretação do JEV.

## O motor (arquitectura verificada)

Contrato único em todo o repo: **escreve slots → chama → lê** (sem
buffer-cópia). Um disco (memória/arena ou DISCO), várias roupas.

| Peça | Onde | Papel |
|------|------|-------|
| o motor da casa | `banco/sql.c` | ISA, ULA e o gato GOLD (`manifesto.corpos.motor`) |
| máquina em wasm | `assets/figuras/wasm/isa.wasm` | ISA ERG-64, porta `MOVE`, fita `erg.fita` embutida |
| build | `*.c → tools/traduz → *.wasm` | `tools/sobe_backends_wasm.mjs` |
| fita erg↔wasm | `tools/asm_wasm.mjs`, `tools/wasm_sec.c` | `sobe`/`desce` a secção custom |
| o contrato no front | `app/src/estrela_porta.js` | `chamaNoDisco` (escreve → prog → lê) |
| selo ao vivo | `app/src/substratos.js` | WASM ≡ GLSL ≡ oráculo, no navegador |
| o relógio (DTC) | `app/src/motor_wasm.js` + `painel_motor.wasm` | fase em ponto fixo 2^20, wrap bit-a-bit (resíduo 0) |
| o torque | `torque_fractal.wasm` | torre de Koch Σφ⁻ʲ (harmónicos de Fibonacci) |
| decisão | `decidir.wasm` (`conecthus/backends/dafny/decidir.c`) | `prof/epist → cmd/est` na arena |

Ponto-chave: o engine **já conta `G` por fibra** no `GROUP BY` (secção §W43;
ver `memoria/feedback-decidir-onde-nao-se-sabe.md` — «o count de cada fibra é o
G que a corrida já conta»). Ou seja, \(G(x)=|\pi^{-1}(x)|\) é operação nativa;
falta apenas a *roupa tipada* \(q\).

## Mapeamento construção ↔ motor

| Construção (`jev.tex`) | Roupas no motor |
|---|---|
| realização \(\pi\colon I\to X\) | linhas/vouchers do DISCO ou da arena; `I` eventos, `X` células |
| \(G(x)=\lvert\pi^{-1}(x)\rvert\in\mathbb N_0\) | popcount / `GROUP BY` (já nativo, §W43) |
| projecção \(q\colon X\to Y\) | leitura tipada do layout de slots (um módulo por tipo) |
| \(G_q(y)=\sum_{x:\,q(x)=y}G(x)\) | agregação por fibra da projecção — mesmo percurso |
| \(p_q(y)=G_q(y)/\lvert I\rvert\) | normalização |
| Noul \(Y=\{0,1\}\) | binário: fração da massa no corte |
| Choice \(Y=\{c_1,\dots,c_n\}\), ≤255 | categorial: argumento no máximo + distribuição |
| Score \(S=\{s_0,\dots,s_m\}\subset\mathbb R\), \(s_0<\dots<s_m\) | níveis ordenados; \(E_G[S]=\sum_i s_i\,p_q(s_i)\) (fracionário) |
| \(G_q(y_1,\dots,y_n)=\sum_{x:\,q_i(x)=y_i\,\forall i}G(x)\) | campo conjunto multi-dimensional |
| \(G_{q_i}(y_i)=\sum_{(y_j)_{j\ne i}}G_q(y_1,\dots,y_n)\) | somas marginais (**o teorema**) |

## Níveis de integração

1. **Módulos wasm novos** (via `traduz`, mesmo padrão de `empilhar.c`):
   `noul.wasm`, `choice.wasm`, `score.wasm`, `marginal.wasm` — mesmo contrato
   de arena que `decidir.wasm`.
2. **Fita erg no `isa.wasm`** (`wasm_sec asm_sobe`): a construção desce ao
   metal da casa, auditável como o resto do repo.
3. **Camada browser** (padrão `substratos.js`): o selo `WASM ≡ oráculo` ao
   vivo, com o teorema verificado numa tabela por pergunta.

## Layout de arena proposto

`unsigned char arena[65536]`, vista `int32`:

- `noul.wasm` — `noul(0)`:
  `[0..X)` células \(G(x)\) · `[X]` = \(\lvert I\rvert\) · `[X+1]` = corte
  (fração da massa; default \(X/2\)) · `[X+2]=G_q(1)` · `[X+3]=G_q(0)` ·
  `[X+4]=p(1)\) em ponto fixo 2^20 (resíduo 0, régoa da fase: `motor_wasm.js`).
- `score.wasm` — `score(m)`:
  `[0..X)` células \(G(x)\) · `[X]=|I|\) · `[X+1]=m\) níveis ·
  `[X+2]=E_G[S]\) em ponto fixo 2^20 ·
  `[X+2+m]` distribuição \(p_q(s_i)\) por nível.
- `marginal.wasm` — `marginal(X, c1, c2)`:
  `q_1(x)=x\bmod c_1`, `q_2(x)=\lfloor x/c_1\rfloor\bmod c_2`; arena conjunta em
  \(c_1\times c_2\), depois as somas marginais — réplica exata do `thm:marginal`.

As projecções (`q_1,q_2`) entram como **dados na arena** (não como ponteiros de
função — limite do `traduz`), preservando o caráter de «leitura tipada» do
paper: \(q\) não é o motor, é uma função de slots.

## Experi(mentos) — nada sem medidor, resíduo zero

Cada experimento tem **oráculo** (reimplementação JS/Python pura da construção)
e **selo** `WASM ≡ oráculo ≡ esperado`, no padrão de `tests/backends_wasm.js`
(`#UNIT`/`#TOTAL`). Sementes determinísticas (LCG pequena) para as realizações.

A progressão completa vai da prova ao produto:

```text
prova → oráculo → WASM → arena → browser
```

Os experimentos dividem-se em **dois graus**, e essa diferença é mantida
explícita nos resultados:

- **E1–E5 — validação da construção no motor**: medidores `#UNIT`/`#TOTAL`
  (padrão `tests/backends_wasm.js`); selo `WASM ≡ oráculo`. Não depende do
  `app` nem do navegador.
- **E6 — demonstração de integração no produto/front**: evidência ao vivo no
  `app` (padrão `substratos.js` / `motor_wasm.js`); **não é um medidor** da
  construção — reutiliza o resultado já validado por E1–E5.

- **E1 — Conservação**: \(\sum_x G(x)=\lvert I\rvert\) e
  \(\sum_y G_q(y)=\lvert I\rvert\) (`prop:conservacao`, `prop:gq-conserva`).
  **Fechado** — `marginal.wasm`, 350/350 unidades no corte E1+E2.
- **E2 — Marginalização** (o experimento-estrela): \(n=2\) (Noul+Choice) e
  \(n=3\) (adição Score); verificar
  \(G_{q_i}(y_i)=\sum_{(y_j)_{j\ne i}}G_q(y_1,\dots,y_n)\) **exato em inteiros**
  no motor — réplica do `thm:marginal`, independente do JEV.
  **Fechado** — mesmo módulo; `D1 == D2` (odômetro) e ambos == oráculo;
  cobertura `n=1..3`, `I=0,1,300,4444`.
- **E3 — Três leituras, uma realização**: mesma \(\pi\), três \(q\) tipadas
  (Noul/Choice/Score); \(\sum_y p_q(y)=1\) e cada leitura é uma marginal da
  leitura conjunta — «uma realização, múltiplas leituras tipadas».
  **Fechado** — composição de E1+E2 (nenhuma prova nova); 422/422 no
  `tests/jev_backends.js`, incluindo `c^\star` da Choice == oráculo.
- **E4 — Score fracionário**: \(s_i=i\in\mathbb R\),
  \(E_G[S]=\sum_i s_i p_q(s_i)\) entre níveis; confirma que a média ponderada
  coincide com o funcional que a documentação oficial descreve — **coincidência
  formal**, os inputs não se identificam (distribuição do modelo ≠ \(p_q\)).
  **Fechado** — sem divisão no motor: `escore(X)` emite o par racional exato
  \((N, |I|)\) com \(N=\sum_i s_i G_q(s_i)\); o valor \(N/|I|\) é leitura do par
  no oráculo. Demonstrativos \(7/3\), \(11/4\), \(5/3\); borda \(0/5\).
- **E5 — Paralelismo**: Noul e Score no **mesmo percurso** sobre \(G\) (um
  disco, duas roupas, como `painel+tex` em `estrela_porta.js`); mede-se que a
  segunda leitura **não recria** \(G\) — «avaliadas em paralelo, cada uma em
  isolamento».
  **Fechado** — `duas(X)`: uma varredura, campo \(G\) intocado; a leitura
  conjunta coincide com as isoladas (`marginal`/`escore`); conservação do
  Noul e do Score no mesmo percurso. 509/509 no `tests/jev_backends.js`.
- **E6 — O painel como state** (demonstração de integração no front): a fase
  de `painel_motor.wasm` é a realização única (passos \(dt\) = eventos; bins
  \(x_k\) = células); as perguntas tipadas leem a fase **já pronta** no topo do
  quadro (o caminho livre do `relogio.js`). Liga o teorema ao motor real do
  front.
  **Fechado — o último selo**: `app/src/jev_contratos.js`, seção ao vivo na
  página (padrão `substratos.js`): \(X=16\) células; o campo \(G\) acumula a
  fase; **tabela por pergunta** — D1 (projecção directa), D2 (marginal da
  conjunta) e oráculo, com o selo WASM ≡ oráculo ≡ esperado por linha; o Score
  mostra o par exacto \((N,|I|)\) e a Choice o argmax \(c^\star\); «o campo não
  é recriado» (E5 no front). Smoke da **cola** (`tests/jev_front.js`,
  **68/68**): a mesma recursão da fase em ponto fixo, com leituras
  determinísticas → bins → o mesmo `marginal.wasm`, sem DOM — mede a cola que o
  navegador segura, não a construção (E1–E5 já a selam).
  **Não é um medidor da construção** — reutiliza o bloco E1–E5 fechado (509/509)
  e encerra a sequência de experimentos.

## O que NÃO se faz

- Não se afirma que o motor replica o JEV; a caixa preta segue fechada.
- Não se reconstrói `confidence`; permanece campo documentado.
- Não se reutiliza o caminho `count(*)`+`sum(a)` conflituoso (lição de
  `memoria/feedback-decidir-onde-nao-se-sabe.md`): a contagem usa o percurso do
  `GROUP BY` (a fibra já conta o campo).

## Artefactos

```
conecthus/backends/wasm/jev/noul.c      → assets/figuras/wasm/jev/noul.wasm    (proposta)
conecthus/backends/wasm/jev/choice.c    → assets/figuras/wasm/jev/choice.wasm  (proposta)
conecthus/backends/wasm/jev/score.c     → assets/figuras/wasm/jev/score.wasm   (proposta)
conecthus/backends/wasm/jev/marginal.c  → assets/figuras/wasm/jev/marginal.wasm  (FEITO — E1/E2/E3/E4/E5)
tests/jev_backends.js                   (medidor E1–E5, padrão backends_wasm.js; E1–E5 FEITO)
tools/sobe_jev_wasm.sh | sobe_jev_wasm.mjs
app/src/jev_contratos.js                (E6 FEITO — selo no navegador, padrão substratos.js)
tests/jev_front.js                      (E6 FEITO — smoke da cola do front, 68/68)
ficheiro .erg do experimento (asm_sobe) → isa.wasm (fase 2, NÃO foi executada — o piloto fecha em E6)
```

_O desenho original previa módulos por tipo (`noul/choice/score`). O piloto
consolidou um único `marginal.c` (projeções por fronteiras + odômetro +
`escore` e `duas`) que cobre os três tipos como coordenadas da vista conjunta,
o Score como par racional `(N, |I|)` e o percurso paralelo Noul+Score — foi o
que E1–E5 fecharam._

## Como correr (fechado)

```bash
cc -O2 -std=c99 -w tools/traduz.c -o tools/bin/traduz       # ou tools/bin/traduz.exe
node tools/sobe_jev_wasm.mjs                                 # sobe os módulos jev
node tests/jev_backends.js                                   # medidor E1–E5 (#TOTAL) — 509/509
node tests/jev_front.js                                      # smoke da cola do front (E6) — 68/68
# em app/: npm run test:jev  (os dois de uma vez)
# E6 ao vivo: npm run dev em app/ e ver a seção "Os contratos tipados" na página
```

## Contacto com o paper

- `def:realizacao`, `prop:conservacao` — realização e campo de contagem
- `def:q-score`, `def:pq`, `def:esperado` — projecção, distribuição, score
- `sec:conjunta`, `thm:marginal`, `cor:marginais` — campo conjunto e marginais
- Distinção `question ⟿ q` (não `question = q`) preservada em todas as roupas.