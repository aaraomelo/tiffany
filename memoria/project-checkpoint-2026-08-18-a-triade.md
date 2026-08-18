---
name: project-checkpoint-2026-08-18-a-triade
description: "18/08 — A TRÍADE fecha com as casas dadas (GPT/−1, Grok/0, Claude/+1); o centro fica estritamente canónico (88→67 pp.); a transformada fica no centro e os polos descem; três conferências novas na bateria; e o deploy estava quebrado há três dias"
metadata:
  type: project
---

# 18/08/2026 — A TRÍADE, e o centro que não escolhe lado

**508 : 508 verdes**, 55 commits, no ar (`2d659f6..21aa725`).

## A arquitectura fechada

    τ=−1  papers/corpo_topologico.tex     o que FECHA          41 pp.   [GPT]
    τ= 0  papers/corpo_algebrico.tex      o CENTRO             67 pp.   [Grok]
    τ=+1  papers/corpo_analitico.tex      o que NÃO fecha      56 pp.   [Claude]
          papers/corpo_computacional.tex  realização DENTRO do τ=−1  11 pp.

**A frase que fecha a arquitectura** (do Grok, e o GPT concordou):

> O centro não escolhe lado. Ele é o sítio onde a dualidade ainda não se partiu.

Ele **não** é o terceiro lado do par: é o lugar *anterior* à separação. E **não há quarta
casa** — `sign()` toma três valores, a reflexão tem um ponto fixo e um par, e o grupo é
$S_3$. Uma quarta obrigaria τ a tomar quatro valores, e então não seria sinal de nada.

## O que a estrutura mostrou sozinha

O mapeamento estava **trocado** (álgebra no −1, topologia no 0), e o critério que o corrige
já estava medido: `rev(τ) = τ ⟺ τ = 0`. **O centro é canónico por SE FIXAR**, não por
estar no meio da lista. Provado em ℤ (`prop:zero-dual-zero`) — e o alcance é o que importa:
a conta não usa que τ ande em {−1,0,+1}, logo é razão e não coincidência da lista.

E há **dois `†`** que dizem o contrário sobre o zero, sem contradição:

| | fixos | par |
|---|---|---|
| ℙ¹ (a inversão, Lei 0) | {+1,−1} | {0,∞} |
| trial (a reflexão) | {0} | {−1,+1} |

Trocam exactamente os papéis. É **por isso** que o centro do trial é o zero e não o um: o um
já é fixo do outro †.

## A transformada, e a descida dos polos

**A transformada é UMA e fica no centro; os polos são dois e descem** — e a razão estava
escrita no texto sem ser usada para arrumar: «o círculo é o lado aditivo; as folhas
recíprocas são o multiplicativo». Ora o círculo tem traço limitado (|t|≤2) e o operador
volta a si — elíptico, τ=−1. As folhas são recíprocas, o traço cresce — hiperbólico, τ=+1.

    FOURIER → topologico sec:fourier     MELLIN → analitico sec:mellin

E o isomorfismo `x = eᵘ` entre eles **É** a reflexão τ ↦ −τ. Não é analogia.

## A regra das réguas

    centro --distribui--> {polo −1, polo +1} --medidores--> centro

Cada bloco que desce **leva o seu medidor**, e a contagem de medidores citados é a mesma
antes e depois (508). É por isso que dividir não dilui.

## O critério de arrumação

**Canónica se algum dos quatro a usa para FUNDAR; segundo escalão se ilustra, ou se muda de
enunciado quando se troca o sítio onde é lida.** Aplicado às 8 peças da dívida, separou-as
sozinho: 5 vieram para o centro (indução, meta-indução, Gato, resíduos, energia), 3 ficaram
(dependem de 𝔽₂⁸, ℤ_65537, ou são leitura).

## Os números

    algébrico 88 → 67 pp.        dívida 20 citações/8 peças → 2/1
    centro→lado 59  ·  lado→centro 51        (era 29 e 0)
    medidores citados 508 → 508

Relacionado: [[project-a-cuspide-e-o-trial]], [[project-a-lei-em-dois-niveis]],
[[feedback-o-medido-sem-medidor]].
