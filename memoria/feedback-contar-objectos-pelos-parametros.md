---
name: feedback-contar-objectos-pelos-parametros
description: "Contei «objectos» pela parametrização para decidir se a leitura era fiel — a pergunta pressuposta na resposta."
metadata:
  type: feedback
---

Escrevi o critério «a leitura serve sse separa» e montei a tabela contando, por corpo,
**objectos** contra **endereços distintos**. O racional reduzido saiu **incompleto**:
56 objectos, 23 endereços. Mas os objectos do corpo racional *são* as 23 classes — eu
tinha contado os 56 **pares**, que é a parametrização.

**Contei objectos pela parametrização para decidir se a parametrização era fiel.** A
pergunta estava pressuposta na resposta, e a tabela deu o veredicto ao contrário: a
única leitura que endereçava o objecto certo foi a que reprovou.

**Why:** «objecto» não é o que eu escrevo no laço `for`. É o que a **igualdade do corpo**
diz que é. Enquanto a igualdade não estiver escrita, `nobj` é uma opinião — e uma
opinião que se disfarça de contagem passa verde.

**How to apply:**

1. **Escrever a igualdade do corpo ANTES de contar.** No racional é `ad = bc`, não
   `(a,b) = (c,d)`. Sem essa linha não há denominador legítimo.
2. **O critério tem DUAS metades e elas são duais** — é o [[feedback-dual-exige-dois]]
   outra vez, e eu tinha escrito só uma:
   - **bem definida**: `x = y` no corpo ⟹ `R(x) = R(y)` — não **quebra** o objecto
   - **separadora**: `R(x) = R(y)` ⟹ `x = y` — não **funde** objectos
   Uma só metade não decide nada: a leitura crua do racional *separa* e é inútil.
3. **As duas falhas têm testemunhas de forma OPOSTA**, e é por aí que se distinguem:
   quebra = *mesmo* objecto em endereços diferentes (`-4/-4` e `-3/-3`);
   funde = objectos *diferentes* no mesmo endereço (`[[-4,-1],[-1,-4]]` e
   `[[-4,1],[1,-4]]`, mesma cifra). Medido em `pgwire.c` §W127.
4. **O par é a prova de que o defeito é da leitura.** Correr a *mesma* família com a
   outra leitura: a reduzida deu 0 quebras e 0 fusões sobre os mesmos objectos. Sem
   esse par eu teria culpado o corpo — que é [[feedback-dois-caminhos]].

Ver [[project-a-navegacao-herda-a-regua]].
