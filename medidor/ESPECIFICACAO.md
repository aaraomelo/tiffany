# Especificação do medidor — ι₁ : X₁ ↪ X₂

## Domínio

X₁ = Z/256Z — 256 entradas, exaustivas: b = 0, 1, …, 255.

## O que mede

Para cada b:
- observação 1: π₁(iota₁(b)) == b  (canal 1)
- observação 2: π₂(iota₁(b)) == 0  (canal 2)

Cada canal é medido SEPARADAMENTE. Não se usa "par == esperado" como único teste.

## Oráculo independente

O esperado é construído da definição de X₂, sem chamar iota₁:
O(b) = (b, 0)

## Controles negativos independentes

1. (b, b) — passa canal 1, falha canal 2, para TODO b (256 casos).
2. (S₁(b), 0) — falha canal 1, passa canal 2, PARA b ∈ {0, …, 254}. S₁(255)=0 invalida o controle; fica fora por periodicidade, não por defeito do medidor.

## Autoteste do instrumento

Antes de medir a implementação, o medidor demonstra que detecta:
- erro só no canal 1
- erro só no canal 2

Se os controles negativos não produzirem os padrões esperados, a implementação não é aceite como evidência.

## Representação

Não decidido ainda. Verificar no paper/código o que está sancionado. Se uint16_t (C_2 do paper), π₁/π₂ por masking/shifts independentes da função iota₁.

## Saída textual determinística

- entradas testadas: 256/256
- resultado π₁
- resultado π₂
- resultado controle negativo 1 (b,b)
- resultado controle negativo 2 (S₁(b),0), b=0..254
- falhas, se houver
- resumo final

## Localização proposta

tests/medidor_iota1.c (junto dos medidores existentes), NÃO medidor/.

## O que NÃO faz

- Não implementa ι₁.
- Não altera campos.tex, espaco.tex, papers.
- Não decide X/F₈.
- Não abre frente da seta.