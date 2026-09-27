# Especificação do medidor — ι₁ : X₁ ↪ X₂

## Domínio

X₁ = Z/256Z — 256 entradas, exaustivas: b = 0, 1, …, 255.

## Representação escolhida

**uint16_t via C₂ do paper**, não par de uint8_t. Evidência:

- `redes/campos.tex:382-395` define C_d(b₀,…,b_{d−1}) = Σ b_k·256^k como bijecção para {0,…,256^d−1}. Para d=2, C₂(b₀,b₁) = b₀ + 256·b₁.
- `CONTRATO_X2.md` §7.2: a codificação de X₂ por inteiro de 16 bits é a própria C₂ do paper, não invenção deste contrato.
- `lib/palavra8.h:23` tem `typedef uint8_t Word8` e `lib/dual16.h:20` tem `typedef struct { uint16_t alto, baixo; } D32`. O projeto já usa uint16_t para pares de largura.

Lido independentemente do construtor iota₁:
- π₁(x) = (uint8_t)(x & 0xFF)
- π₂(x) = (uint8_t)((x >> 8) & 0xFF)

## O que mede

Para cada b:
- observação 1: π₁(iota₁(b)) == b  (canal 1)
- observação 2: π₂(iota₁(b)) == 0  (canal 2)

Cada canal é medido SEPARADAMENTE. Não se usa "par == esperado" como único teste.

## Oráculo independente

O esperado é construído da definição de X₂, sem chamar iota₁:
O(b) = (b, 0)  →  código: (uint8_t)b, (uint8_t)0

## Controles negativos independentes

1. (b, b) — passa canal 1, falha canal 2, para TODO b (256 casos).
2. (S₁(b), 0) — falha canal 1, passa canal 2, PARA b ∈ {0, …, 254}. S₁(255)=0 invalida o controle; fica fora por periodicidade, não por defeito do medidor.

## Autoteste do instrumento

Antes de medir a implementação, o medidor demonstra que detecta:
- erro só no canal 1
- erro só no canal 2

Se os controles negativos não produzirem os padrões esperados, a implementação não é aceite como evidência.

## Saída textual determinística

- entradas testadas: 256/256
- resultado π₁
- resultado π₂
- resultado controle negativo 1 (b,b)
- resultado controle negativo 2 (S₁(b),0), b=0..254
- falhas, se houver
- resumo final

## Localização

`tests/medidor_iota1.c` — junto dos medidores existentes.

## O que NÃO faz

- Não implementa ι₁.
- Não altera campos.tex, espaco.tex, papers.
- Não decide X/F₈.
- Não abre frente da seta.