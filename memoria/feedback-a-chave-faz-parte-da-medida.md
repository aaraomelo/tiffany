---
name: feedback-a-chave-faz-parte-da-medida
description: "O prefixo de arrumação que eu pus nas falas valia 0,142 de cosseno — a métrica media a minha convenção, não o material"
metadata: 
  node_type: memory
  type: feedback
  modified: 2026-08-02T17:13:52.694Z
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
---

# A chave faz parte da medida

02/08/2026, ao crescer o tecido da assistente de 830 para 1701 pares com o catálogo e o enredo.

**O que aconteceu.** Eu tinha publicado no catálogo que o cosseno médio entre pares "desce
monotonamente" ao crescer o tecido, e que isso **provava** crescimento em diversidade. Era uma lei
declarada sobre **quatro pontos**. O quinto subiu: $0{,}4798 \to 0{,}5221$.

Verifiquei primeiro que não era artefacto de amostragem — mesmo procedimento (24 amostras, passo
uniforme) nos dois tamanhos, a subida ficou. Depois medi as camadas em separado, e o resultado foi
o **oposto** do que eu supunha: o culpado não era o enredo (685 pares de narrativa contínua, que eu
achava homogénea) mas o **catálogo** — 0,8261, a camada mais homogénea de todas, quando é o
documento mais diverso do repositório.

**A causa era minha.** Eu tinha prefixado todas as falas novas com `catálogo: ` e `enredo: `, para
saber de onde vinham. Isolando o efeito nas mesmas 186 entradas:

    a fala como eu a escrevi    "catálogo: tools/mmu.c"    0,8261
    sem o prefixo               "tools/mmu.c"              0,6840
    a tese (o conteúdo real)                               0,6495

**O prefixo constante vale 0,142 de cosseno e não carrega informação nenhuma** — é a mesma cadeia
em todas as falas da camada, e o embedding conta-a como semelhança. Refeito o tecido sem ele:
0,4863, praticamente a diversidade que havia a 830, com o dobro do material.

**Why:** a métrica não estava a medir o material — estava a medir uma convenção de arrumação que eu
tinha introduzido no mesmo passo. E como a convenção era invisível para mim (era "só" um prefixo
para me orientar), o número saiu credível: subiu pouco, na direção plausível de "material novo é
mais parecido entre si". Se eu não tivesse ido isolar, teria escrito uma segunda lei falsa por cima
da primeira. É a irmã da [[feedback-assercoes-vazias]]: ali a asserção não podia falhar, aqui a
medida não podia dizer a verdade.

**How to apply:** antes de comparar duas medições, perguntar **o que mudou além do material** — o
formato da chave, o prefixo, o separador, o corte do texto. Se as entradas partilham qualquer
cadeia constante que eu escolhi, medir com e sem ela antes de interpretar a diferença. E a regra
geral, que serve para além dos embeddings: **quando a métrica melhora ou piora ao mudar de camada,
suspeitar primeiro do que eu acrescentei na camada, não do que o material é**. Liga com
[[feedback-verdadeiro-e-parcial]] (a lei declarada sobre metade dos pontos) e com
[[feedback-simulacao-nao-bate]] (as escalas e convenções antes da lógica).
