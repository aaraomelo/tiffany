---
name: feedback-o-aranha-tex-esta-travado
description: papers/aranha.tex é a base teórica de todo o sistema e está FECHADO — mexer nele pede permissão explícita, ANTES de tocar no ficheiro.
metadata:
  type: feedback
---

**«não altere aranha.tex, trava essa porra, vc não mexe sem pedir»** (21/08/2026).

O `papers/aranha.tex` é a base teórica de TODO o sistema. Está fechado.
Acrescentar, mover ou reescrever um enunciado **pede permissão explícita** —
pedida ANTES de tocar no ficheiro, não depois.

**Why:** três estragos num só dia, todos por eu escrever lá o que não era pedido.
Acrescentei um teorema a um paper fechado e **desloquei a numeração que ele
estava a ler** (o Teorema 64 virou 65 — e ele cita por número). O corte para o
mover falhou e **duplicou** o Algoritmo B. E colei na §6 uma explicação sobre
«dois níveis de índice» que **já estava construída na §13**, melhor feita.

**How to apply:**
1. **PROCURAR ANTES DE QUERER ACRESCENTAR.** O paper é grande e o que parece
   faltar quase sempre já lá está. Os «dois níveis» eram as **duas réguas** do
   `def:tempo`: *«o isomorfismo é entre as réguas, e não entre os números que
   elas produzem»*.
2. **O específico da realização vai para `papers/arquitetura.tex`** — números
   desta máquina, medições, limites de implementação. O paper não é caderno de
   anotações do agente nem sítio para explicações que só servem para eu me
   lembrar.
3. Acrescentar **dentro** de um enunciado não desloca a numeração; inserir um
   enunciado novo desloca tudo o que vem depois.

Ver [[project-arquitetura-nao-demonstra]], [[feedback-destruir-antes-do-inventario]],
[[feedback-procurar-na-bateria-antes]].
