# opt_caelum — Descrição Matemática e Script GLSL

## Card
`opt_caelum` — 6s, 24fps, 640×640 (gravado em `gifs/opt_caelum_6s_24fps.gif`)

## Equação aplicada

$$a coroa costurada + os 6 efeitos ópticos (PBR)$$

## Técnica
Ver manifesto

## Script GLSL Aplicado

O GLSL é gerado dinamicamente por `cards_kernel.js` → `cards_campo.js`. O kernel do opt_caelum aplica:

```glsl
// opt_caelum: a coroa costurada + os 6 efeitos ópticos (PBR)
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

// opt_caelum: n=8
float n = 8.0;
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

### Textura (La Hire ⊗)

$$\cos 8(\theta - \gamma)$$

- n=8: 8 pétalas
- θ: azimute do ponto
- γ: fase do motor (varia com o tempo)

### Óptica (PBR — 6 efeitos)

| Efeito | Equação | Técnica |
|--------|---------|---------|
| Difuso | I = max(n·L, 0) · ρ/π | Lambert |
| Especular | spec = max(n·H, 0)^s | Blinn-Phong |
| AO | ao = 1 − Σ(h − sdf(p+n·h))·scaⁱ | SDF |
| Fresnel | F = F₀ + (1−F₀)(1−cos θ)⁵ | Schlick |
| Sombra | res = min(1, k·sdf(p+L·t)/t) | Soft shadow |
| Reflexão | R = V − 2(V·n)n | Especular |

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
- gifs/opt_caelum_6s_24fps.gif — GIF gravado
