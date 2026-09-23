# Benjamim @ Elenco3d — Descrição Matemática e Script GLSL

## Card
`benjamim@elenco3d` — 6s, 24fps, 640×640 (gravado em `gifs/benjamim@elenco3d_6s_24fps.gif`)

## Equação aplicada

$$\prod = \exp \circ \Sigma \circ \log$$

O produto de Pontryagin revela o atrator via logaritmo: o logaritmo converte o produto em soma ($\log(ab) = \log a + \log b$), a soma $\Sigma$ opera no espaço dual, e a exponencial reconverte.

## Técnica

- Logaritmo: $\log: (X, \otimes) \to (X, \oplus)$ — transporta o produto para a soma
- Soma no dual: $\Sigma$ opera em $X^*$ pelo emparelhamento $\langle a, b \rangle = \text{paridade}(a \wedge b)$
- Exponencial: $\exp: (X, \oplus) \to (X, \otimes)$ — reconverte ao corpo original
- O produto $\prod$ é a realização canónica de $\mathcal{D}$ no card: troca $(\oplus, \otimes) \mapsto (\otimes, \oplus)$

## Script GLSL Aplicado

O GLSL é gerado dinamicamente por `cards_kernel.js` → `cards_campo.js`. O kernel do Benjamim aplica:

```glsl
// Benjamim: produto = exp ∘ Σ ∘ log
// Fragment shader — Benjamim "O Quiral" (n=3, S¹ · ℤ/3)

uniform float uTime;
uniform vec2  uResolution;
uniform float uPhase;   // fase do motor (0..1 por ciclo)

// --- Funções auxiliares ---
float log_(float x) { return log(max(x, 1e-7)); }
float exp_(float x) { return exp(x); }
float sigma_(float x) { return mod(x, 1.0); }  // soma no dual [0,1)

// --- Produto de Pontryagin ---
float produto(float a, float b) {
    float logA = log_(a);
    float logB = log_(b);
    float sum = sigma_(logA + logB);  // Σ no dual
    return exp_(sum);                  // exp de volta
}

// --- Coordenadas polares ---
vec2 center = vec2(0.5);
float r = length(gl_FragCoord.xy - center * uResolution) / (0.5 * uResolution.y);
float theta = atan(gl_FragCoord.y - center.y * uResolution.y,
                   gl_FragCoord.x - center.x * uResolution.x);

// --- Benjamim: n=3, 3 pétalas ---
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

## Transformação Período (6s, sem descontinuidade)

O período de 6 segundos cobre o ciclo completo da fase do motor:

| Tempo (s) | Fase do motor | Transformação |
|-----------|---------------|---------------|
| 0.0 – 1.5 | $\phi \to \varphi$ | Log: $\log(ab) = \log a + \log b$ |
| 1.5 – 3.0 | $\Sigma$ no dual | Soma: $\sigma_1 = \varphi$ |
| 3.0 – 4.5 | $\exp$ de volta | Exp: $\exp(\Sigma(\log))$ |
| 4.5 – 6.0 | $\prod$ revela | Produto: $\prod = \exp \circ \Sigma \circ \log$ |

A transição é contínua porque cada fase é interpolada linearmente sobre o ciclo de 6s, e a função `produto` é aplicada ponto-a-ponto sem saltos.

## Equações dos Efeitos de Textura e Óptica

### Textura (La Hire ⊗)

$$\cos 3(\theta - \gamma)$$

- $n=3$: 3 pétalas
- $\theta$: azimute do ponto
- $\gamma$: fase do motor (varia com o tempo)

### Óptica (PBR — 6 efeitos)

| Efeito | Equação | Técnica |
|--------|---------|---------|
| Difuso | $I = \max(\mathbf{n}\cdot\mathbf{L}, 0) \cdot \rho/\pi$ | Lambert |
| Especular | $\text{spec} = \max(\mathbf{n}\cdot\mathbf{H}, 0)^s$ | Blinn-Phong |
| AO | $\text{ao} = 1 - \sum (h - \text{sdf}(p+n\cdot h))\cdot\text{sca}^i$ | SDF |
| Fresnel | $F = F_0 + (1-F_0)(1-\cos\theta)^5$ | Schlick |
| Sombra | $\text{res} = \min(1, k\cdot\text{sdf}(p+L\cdot t)/t)$ | Soft shadow |
| Reflexão | $\mathbf{R} = \mathbf{V} - 2(\mathbf{V}\cdot\mathbf{n})\mathbf{n}$ | Especular |

## Notas

- O GLSL acima é **pseudocódigo** — o script real em `grava_todos_cards.js` captura o canvas do navegador, não compila GLSL.
- O GLSL real está em `app/src/cards_kernel.js` (WebGL2) e `app/src/cards_campo.js` (campo de shader).
- O card `benjamim@elenco3d` usa `smooth-min` como dual, $n=3$, anel $S^1 \cdot \mathbb{Z}/3$.
- $\operatorname{FP}=1 \cdot \text{Koch N=5 casada}$ (fator de potência).

## Arquivos Referenciados

- `app/src/manifesto.json` — definição do card
- `app/src/cards_kernel.js` — kernel WebGL2
- `app/src/cards_campo.js` — campo de shader
- `app/src/banco_relogio_u.js` — fase do motor
- `gifs/benjamim@elenco3d_6s_24fps.gif` — GIF gravado