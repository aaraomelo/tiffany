---
name: project-aranha-em-zn
description: O thm:multiplicidade não usa o 2 — a aranha corre em Z^n, na ultramétrica, e o campo G lê o período da órbita.
metadata:
  type: project
---

`tests/aranha_n.c` §AN1–§AN9, `arquitetura.tex` Cor. `cor:aranha-n` e
`cor:aranha-inversa-n` (20/08/2026, commit e93c7ac).

**O 2 era do exemplo, não do teorema.** As cláusulas 1–3 do
`thm:multiplicidade` são sobre a FIBRA de uma aplicação: `i∼j ⟺ π(i)=π(j)`,
`G=|π⁻¹|` conta-a, e `G_{t+1}=G_t+1_{π(t)}` é escrita local no contradomínio.
Nenhuma menciona a dimensão. **Só a cláusula 4 a usa**, e como `|V(x)|=2n`.
Escrito com n por parâmetro, o mesmo código corre ℤ¹–ℤ⁴: Cantor, dragão,
dragão no espaço, o passo da ISA (registo,slot,banco,andar). O campo é
esparso num arena fixo — a cláusula 3 lida à letra, e sem RAM.

**Três coisas que a casa não tinha escrito:**

1. **A ultramétrica não é uma desigualdade mais apertada**: é as bolas
   PARTICIONAREM e todo ponto de uma ser seu centro. A célula de π **É** a
   bola, e ∑G=|I| sem sobreposição porque não há sobreposição a haver. A
   recta com o mesmo raio falha as duas. Varrer `d ≤ max(d,d)` seria
   substituir a prova (três casos do primeiro bit divergente) pelo número.
2. **O campo G lê o PERÍODO.** Órbita determinista (z↦z²+c em ℤ[i]): a célula
   É o estado, logo `|{x : G(x)>1}| = período` e `{G=1}` = a cauda. O ρ de
   Floyd lido no chão — o clássico precisa de duas patas porque o agente não
   tem memória; a aranha tem uma, porque a memória é do espaço.
3. **A inversa sobe UM andar só.** π̃=(π,k) leva ℤⁿ em ℤⁿ⁺¹ — a folha é UMA
   coordenada, não n — e a injectividade não se verifica par a par: é a
   própria aranha em dimensão n+1. E `pr₁` não é um passo que se escreva, é o
   campo em dimensão n a não olhar para a folha. A VOLTA fecha: sem o campo
   da base, recontar devolve G, resíduo 0.

**O DRAGÃO É A DOBRA DA TORRE.** `D_{k+1} = D_k + D_k*`, com `*` = a curva ao
contrário rodada de 90° em torno do ponto final, bate ponto a ponto com a
régua de bits `drag_esq` em k=1..12. É `T_{k+1}=T_k+T_k*` (ver
[[project-a-lei-em-dois-niveis]]) a aparecer numa curva. O sinal da rotação
não se escolhe: correm-se os dois e exige-se que exactamente um bata, o mesmo
em todas as ordens.

**O que NÃO se afirma:** `max G` não compara dimensões. O dragão de ℤ³ dá 3
contra 2 do plano — MAIS, não menos — e a razão é a curva ser outra.
Ver [[feedback-o-invariante-que-nao-separa]].
