---
name: checkpoint-2026-08-11
description: "11/08 — o dia da COMPOSIÇÃO DINÂMICA; commit cf30baa [skip ci], sem deploy"
metadata: 
  node_type: memory
  type: project
  originSessionId: 0d72a882-4e0b-4eb2-bdba-8a2376988e06
  modified: 2026-08-11T19:48:05.432Z
---

**Commit cf30baa** (push em master, `[skip ci]`, deploy adiado a pedido). Bateria **391:391**,
tex.c 59:59, traduz_volta 32 (na bateria), tex_wasm 4:0, volta byte a byte resíduo 0.

**As leis que entraram no tradutor (tudo dinâmico, só as sementes fixas):**
- a fórmula não herda o itálico do texto (mat_entra/mat_sai nas 3 portas; 288 glifos);
- `\\` é escape de UM caractere no avalia_macros (o «mathbbF» da matriz);
- a fronteira compõe-se da assinatura à medida da região — régua única `matriz_regiao`
  (lei 8 dual) para o ramo que pinta + os 3 vãos; o ∫ estica pelo vão;
- os limites pelo giro comum: dilatação→TRANSLAÇÃO (transformada dourada), escala no
  degrau, par no mesmo eixo com a semente de espaçamento propagada;
- sementes na config: `\gksemente` no estilo → `le_semente` → `SEM_V[5]`; `sem_eixo`
  substituiu o 3/10 escrito à mão em 3 sítios.

**traduz.c:** `loc_poe` (o transbordo de MAX_LOC corrompia `nloc` em silêncio — módulo
inválido com linha de erro errada); MAX_LOC 512; inicializador de array local `{…}` NÃO
está no subconjunto (nem `getenv`); a linha de erro do traduz não conta `\n` dentro de
strings — desconfiar dela e instrumentar `erro()` com a vizinhança de POS.

**Os medidores novos:** §X16 (7 unidades: itálico, `\\`, integral, semente, par, dinâmica
da região), §X17 (o λ dualizado no tex↔PDF com mutação no corpo que viaja), §V10 (o
gerador matricial A_m: Parseval dourado det(A^k)=(−1)^k nível a nível DENTRO do módulo;
Lei 0 matricial gato/esquilo; estrutura módulo=Σ), §V11 (λ⁺ em bits, λ⁺+λ⁻=0 60/60; π
sai da dobra 2 bits/dobra; dimensão fecha em erro·c_n<½ — primeira é a 4; a vírgula-fixa
S=2³⁰ esgota ~2⁹ dobras — a história do α). Sob o ulimit da bateria o motor não instancia
e os §V correm pelo cc (dois caminhos).

**Pendente:** deploy (o push foi [skip ci]); o wasm vermelho de ontem ficou VERDE.
Ver [[compor-nao-ancorar]] — as três voltas do erro de composição deste dia.
