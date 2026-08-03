---
name: project-checkpoint-2026-08-03-tarde
description: "03/08 tarde: a bijeção Z×N*↔R e a cifra; e DEZASSETE asserções que não podiam falhar, cinco delas correções minhas do mesmo dia"
metadata:
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
---

03/08/2026, tarde e noite. **74 commits.** Segue-se a [[project-checkpoint-2026-08-03]] da manhã.

## O estado, e está no ar

| | | |
|---|---|---|
| `teoria.tex` | 93 pp. | + o degrau, Cantor, ℤ[i], a continuação |
| `catalogo.tex` | 314 pp. | |
| `enredo.tex` | 298 pp. | zero fórmulas |
| `livro.tex` | **709 pp.** | |

**283 medidores, 280 verdes.** As 3 falhas são todas ambientais: `fita.c` mede *velocidade*,
`tatoeba/regua.c` precisa do corpus, `protocolo.c` precisa do ollama. Os quatro PDFs servidos
com 200 na Patria.

## O resultado

**Z × N\* ↔ R** — todo real é o **limite de uma órbita** de operações de inteiros, e cada passo
inverte-se nos inteiros. `M_w = A_{a₀}···A_{a_{k−1}}`, ida por Möbius, volta por Euclides.

- os **pontos fixos** são só os quadráticos (se `M ∈ GL₂(ℤ)` fixa `x`, então `x` resolve uma
  quadrática inteira; π é transcendente e fica de fora **por um teorema**);
- **ℝ = ℤ × ℕ^{≤∞}**, e não `ℕ^ℕ` — as palavras *finitas* dão ℚ, que é denso;
- **a mesma matriz é a cifra**, mas `det=±1` é *um facto a servir dois papéis*, e não a mesma
  condição: no degrau é teorema, na cifra pode falhar;
- e **a cifra guarda contas, não guarda segredos** — dois pares claro/cifrado entregam a chave
  (é Hill, 1929). Ver [[feedback-o-sujeito-da-frase]]: o que é próprio é o enquadramento por ν
  e a realização executável, e dizer isso é mais forte que reivindicar Khinchin.

Sete medidores novos: `palavra.c` (33 un.), `continua.c` (21), `cantor.c` (17), `gauss.c` (14),
`nomeia.c` (a ferramenta, CLI), `recaman.c`, `refs.c` — e **`mutacao.sh`**.

## O que a tarde ensinou, e é o que fica

**Dezassete asserções que não podiam falhar**, num dia. **Cinco delas eram correções minhas de
horas antes.** O `§P4` teve três versões, duas vazias — e a segunda foi a *correção* da primeira.

Está tudo em [[feedback-assercoes-vazias]]. As três coisas novas:

1. **`tools/mutacao.sh`** — estragar o código e ver se a bateria acusa. Responde ao que 283
   medidores verdes não respondem: *eles medem, ou só correm?*
2. **Uma asserção pode ser vazia como afirmação e ser o único teste de regressão de um bloco.**
   Apaguei uma vazia e abri um buraco: mutei a decifra e tudo ficou verde.
3. **`grep` pela frase EXATA depois de corrigir.** A mesma frase falsa vivia em dois sítios e eu
   corrigi um — a segunda estava no *mesmo ficheiro*, cem linhas abaixo.

E o defeito mais instrutivo do dia: `if(k >= KMAX) continue;  /* não truncar em silêncio */` —
**o comentário estava na linha que truncava em silêncio.**

## Os revisores

Ver [[feedback-revisores-externos]] — repicar é obrigatório, e a segunda ronda precisou de
**uma pergunta só** por agente. O melhor achado deles foi contra a minha própria ferramenta: o
`mutacao.sh` dava falso "SOBREVIVEU" em **541 de 557** medidores, porque lia o rodapé impresso
em vez do código de saída. Falso alarme numa ferramenta de auditoria é pior que não a ter.

## O que fica aberto

- as 3 secções de exposição do catálogo (38 subsecções) continuam sem medidor;
- `256 de 557` medidores sinalizam falha no exit — nos outros a mutação é **inconclusiva**;
- binários antigos no git (`enredo_completo.pdf`, `regua_*.bin`) — anteriores a esta sessão.
