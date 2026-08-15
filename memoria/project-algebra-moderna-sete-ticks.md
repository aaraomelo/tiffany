---
name: project-algebra-moderna-sete-ticks
description: O andar da álgebra moderna na assistente — a espinha de sete ticks que ele exigiu para todo teorema, com a definição em LaTeX no seu lugar.
metadata:
  type: project
---

Commit `81a69e8`, `lib/estrutura.h` + §C35, vinte itens (`algebra` dá o índice). Zero
doubles.

## A ESPINHA, e é ela a entrega

Ele fechou o `eval.txt` com a exigência de **forma**, e ela vale para todos os teoremas:

> hipóteses → definição → transição → **lei usada** → **testemunha** → conclusão → volta

e com a razão: **«o mais importante para o teu sistema: NÃO MEDIR SÓ A CONCLUSÃO».** Os
sete lugares têm de estar preenchidos — a lei que autoriza a transição diz-se pelo nome, e
a testemunha exibe-se. Um teorema que salte a testemunha fica com um buraco visível.

E ele pediu a **definição em LaTeX**: a membrana passa a carregar a matemática, não só o
texto. Antes de escrever, medi que `\iff` e `\triangleleft` NÃO estão no dialecto do
tradutor — usei `\Leftrightarrow` e escrevi a normalidade pela definição (`gkg^{-1} \in K`),
que informa mais que o símbolo. Ver [[feedback-medir-so-metade-do-par]].

**Medir a cadeia e não o fim** mudou o código: na unicidade do inverso os cinco elos de
`b = b⋆e = b⋆(a⋆c) = (b⋆a)⋆c = e⋆c = c` têm asserção cada um; em Lagrange mede-se a
PARTIÇÃO (mesmo tamanho, disjuntas, a cobrir) e a divisibilidade é o que sobra; no primeiro
teorema do isomorfismo a bijeção é **procurada** exaustivamente, não declarada.

## O que o andar diz sobre a casa

«(G,⋆) é grupo quando todo a possui inverso — **exatamente a reversibilidade que vocês já
encontraram no andar dos inteiros**». A folha `a⁻¹ = −a` e o inverso do grupo são a MESMA
condição; o que a álgebra acrescenta é poder exigi-la **sem dizer de que números se fala**.
Este andar não trouxe motor novo: trouxe o nome do que já corria.

E o `eˣ` do §9 pedia decimais. O conteúdo é algébrico — a potência troca + por × — e
realiza-se exato no finito: **(ℤ_{p−1},+) ≅ (ℤₚ*,×)** por `x ↦ gˣ` com g raiz primitiva, e a
volta é o **logaritmo discreto** no lugar do ln. Mesma manobra da série de Dirichlet formal
([[project-mobius-e-elipticas]]): guardar o conteúdo e trocar a realização.

## Os três ramos que não corriam

A mutação apanhou-os, e a causa é a mesma família de [[feedback-varrer-onde-nada-pode-falhar]]:

1. **ℤ₁₂ é ABELIANO** — todo subgrupo é normal e `gH = Hg` sempre. As mutações «normal sem
   conjugar» e «classe do outro lado» sobreviviam à varredura inteira. Entrou **S₃**.
2. **Todos os `x ↦ kx` de ℤ₁₂ são homomorfismos** — o filtro nunca excluía ninguém e podia
   devolver sempre verdade. Entrou `x ↦ x²`, que não é.
3. **Eu escrevia «corpo ⟹ domínio tem um sentido só» varrendo ℤₘ**, onde os dois lados
   coincidem (todo domínio FINITO é corpo): estava a chamar «um sentido» a uma
   equivalência. A direção mede-se em ℤ.

**A regra que daqui sai**: quando o teorema fala de uma assimetria (normal vs. não normal,
um sentido vs. equivalência, preserva vs. não preserva), **o exemplo tem de ser assimétrico**.
Medir num objeto simétrico é medir onde o teorema não distingue nada.
