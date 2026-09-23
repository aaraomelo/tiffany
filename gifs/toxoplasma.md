# toxoplasma — Descrição Matemática e Script GLSL

## Card
`toxoplasma` — 6s, 24fps, 640×640 (gravado em `gifs/toxoplasma_6s_24fps.gif`)

## Equação aplicada

$$o campo de Benjamim, revertido pelo Venom (χ = −1)$$

## Técnica
venom_rev — o processo do Venom (χ = −1)

## Script GLSL Aplicado

O GLSL é gerado dinamicamente por `cards_kernel.js` → `cards_campo.js`. O kernel do toxoplasma aplica:

```glsl
// toxoplasma: o campo de Benjamim, revertido pelo Venom (χ = −1)
// Fragment shader — pseudocódigo

uniform float uTime;
uniform vec2  uResolution;
uniform float uPhase;   // fase do motor (0..1 por ciclo)

// --- Funções auxiliares ---
float log_(float x) { return log(max(x, 1e-7)); }
float exp_(float x) { return exp(x); }
float sigma_(float x) { return mod(x, 1.0); }

// --- Produto de Pontryagin ---
float produto(float a, float b) {
    float logA = log_(a);
    float logB = log_(b);
    float sum = sigma_(logA + logB);
    return exp_(sum);
}

// --- Coordenadas polares ---
vec2 center = vec2(0.5);
float r = length(gl_FragCoord.xy - center * uResolution) / (0.5 * uResolution.y);
float theta = atan(gl_FragCoord.y - center.y * uResolution.y,
                   gl_FragCoord.x - center.x * uResolution.x);

// toxoplasma: n=3
float n = 3.0;
float cosTerm = cos(n * (theta - uPhase));
float rEq = pow(phi, 2.0 * theta / 3.14159265);

// --- Aplicação do produto ---
float a = rEq * cosTerm;
float b = 1.0 - a;
float result = produto(a, b);

// --- Coloração ---
vec3 cor = vec3(0.0, 0.8, 1.0) * result  // ciano (∏)
         + vec3(1.0, 0.84, 0.0) * (1.0 - result); // ouro

gl_FragColor = vec4(cor, 1.0);
```

## Período de Transformação (6s, sem descontinuidade)

| Tempo (s) | Fase do motor | Transformação |
|-----------|---------------|---------------|
| 0.0 – 1.5 | φ → ϕ | Log: log(ab) = log a + log b |
| 1.5 – 3.0 | Σ no dual | Soma: σ₁ = φ |
| 3.0 – 4.5 | exp de volta | Exp: exp(Σ(log)) |
| 4.5 – 6.0 | ∏ revela | Produto: ∏ = exp ∘ Σ ∘ log |

## Efeitos de Textura e Óptica

## Contrato

- O catálogo **registra**; não decide o que é canónico.
- Star(U)=D — troca (⊕,⊗)↦(⊗,⊕).
- Não confundir com Star(K)={x:x²=x+1} nem com factor/métrica local.
- não localizada ≠ N/A: procurar primeiro, demonstrar ausência depois.

## Arquivos Referenciados

- app/src/manifesto.json — definição do card
- app/src/cards_kernel.js — kernel WebGL2
- app/src/cards_campo.js — campo de shader
- app/src/banco_relogio_u.js — fase do motor
- gifs/toxoplasma_6s_24fps.gif — GIF gravado
