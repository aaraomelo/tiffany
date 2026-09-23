# Técnicas Matemáticas por Card — Reino Dourado

Documento gerado a partir de `app/src/manifesto.json` e dos cards gravados em `gifs/`.

## Contrato
- O catálogo **registra**; não decide o que é canónico.
- O canónico estrutural de $\mathcal{U}$ é $\operatorname{Star}(\mathcal{U})=\mathcal{D}$ (troca $(\oplus,\otimes)\mapsto(\otimes,\oplus)$).
- Não confundir $\operatorname{Star}(\mathcal{U})$ com $\operatorname{Star}(K)=\{x:x^2=x+1\}$ nem com factor/métrica local.
- `não localizada` ≠ `N/A`: procurar primeiro, demonstrar ausência depois.

## Cards com GIF gravado (115 total)

### Elenco3d (16 cards)

| Card | Equação aplicada | Técnica |
|------|------------------|---------|
| rainha_tiffany | $\varphi^2 = \varphi + 1$ | Proporção áurea — soma igual ao produto |
| rei_cifra | $\sigma_m = m + 1/\sigma_m$ | Ponto fixo — equação funcional |
| benjamim | $\prod = \exp \circ \Sigma \circ \log$ | Logaritmo converte produto em soma |
| rainha_gelo | $\chi = -1$ (Venom inverte) | Dualidade — negativo do dourado |
| venom | Expansão de de Sitter $r \times 2^{DS\cdot a}$ | Cosmologia — expansão exponencial |
| toxoplasma | $\chi = -1$ (Venom inverte Benjamim) | Dualidade — campo revertido |
| regente_cristalino | Simetria de ordem 6 | Grupo cíclico $\mathbb{Z}/6$ |
| dark_pontryagin | $\sigma_m' = -1/\sigma_m$ | Inverso negativo — dual de Pontryagin |
| joaquim | $\oplus$ (soma, momentum) | Adição — dual do produto |
| yasmin | $\otimes$ (produto, escala) | Multiplicação — dual da soma |
| dafny | Decide o decidível | Lógica — decidibilidade |
| isabelle | Constrói o infinito | Indução — infinito construtivo |
| pegaso | $s^2 + c^2 = 1$ | Identidade pitagórica — rotor |
| merlin | Mellin (escala-invariante) | Transformada de Mellin — invariância de escala |
| fourier | Ondas | Série/Fourier — decomposição freqüencial |
| ada | Gerar ($\oplus$, Corpo Criativo) | Soma — geração |

### Espirais (6 cards)

| Card | Equação aplicada | Técnica |
|------|------------------|---------|
| aurea_mae | $r = \varphi^{2\theta/\pi}$, 2 braços | Espiral áurea — coordenadas polares |
| naturais | $r = \varphi^{2\theta/\pi}$, 1 braço | Espiral áurea — corpo $\mathbb{Z}$ |
| racionais | $r = \varphi^{2\theta/\pi}$, 3 braços | Espiral áurea — corpo $\mathbb{Q}$ |
| continuo | $r = \varphi^{2\theta/\pi}$, 5 braços | Espiral áurea — corpo $\mathbb{R}$ |
| triade | $r = \varphi^{2\theta/\pi}$, 3 braços | Espiral áurea — corpo universal |
| combinacao | $r = \varphi^{2\theta/\pi}$, 8 braços | Espiral áurea — produto de corpos |

### Textura (16 cards)

| Card | Equação aplicada | Técnica |
|------|------------------|---------|
| rei_cifra_textura | $\hat{u} = p/|p|$ (Clifford $\oplus$) → estereográfica | Projeção estereográfica — Clifford $\oplus$ |
| rainha_textura | $\hat{u}=p/|p| \cdot \cos 6(\theta-\gamma)$ (La Hire $\otimes$) | La Hire — produto tensorial |
| tex_benjamim | $\cos 3(\theta-\gamma)$ | Fourier angular — 3 pétalas |
| tex_dark_pontryagin | $\cos 3(\theta-\gamma)$ | Fourier angular — 3 pétalas |
| tex_joaquim | $\cos 2(\theta-\gamma)$ | Fourier angular — 2 pétalas |
| tex_yasmin | $\cos 5(\theta-\gamma)$ | Fourier angular — 5 pétalas |
| tex_dafny | $\cos 4(\theta-\gamma)$ | Fourier angular — 4 pétalas |
| tex_isabelle | $\cos 6(\theta-\gamma)$ | Fourier angular — 6 pétalas |
| tex_pegaso | $\cos 2(\theta-\gamma)$ | Fourier angular — 2 pétalas |
| tex_merlin | $\cos 1(\theta-\gamma)$ | Fourier angular — 1 pétala |
| tex_fourier | $\cos 8(\theta-\gamma)$ | Fourier angular — 8 pétalas |
| tex_regente_cristalino | $\cos 6(\theta-\gamma)$ | Fourier angular — 6 pétalas |

### Óptica (7 cards)

| Card | Equação aplicada | Técnica |
|------|------------------|---------|
| optico_difuso | $I = \max(\mathbf{n}\cdot\mathbf{L}, 0) \cdot f_r = \rho/\pi$ | Lambert — BRDF isotrópico |
| optico_especular | $\text{spec} = \max(\mathbf{n}\cdot\mathbf{H}, 0)^s$, $\mathbf{H}=(\mathbf{L}+\mathbf{V})/|\mathbf{L}+\mathbf{V}|$ | Blinn-Phong — meio-vetor |
| optico_oclusao | $\text{ao} = 1 - \sum (h - \text{sdf}(p+n\cdot h))\cdot\text{sca}^i$ | Ambient Occlusion — SDF ao longo da normal |
| optico_fresnel | $F = F_0 + (1-F_0)(1-\cos\theta)^5$ | Schlick — reflectância de borda |
| optico_sombra | $\text{res} = \min(1, k\cdot\text{sdf}(p+L\cdot t)/t)$ | Soft shadow — marcha até a luz |
| optico_reflexo | $\mathbf{R} = \mathbf{V} - 2(\mathbf{V}\cdot\mathbf{n})\mathbf{n}$ | Reflexão especular — vetor normal |
| optico_completo | Difuso + Especular + AO + Sombra + Fresnel + Reflexão | PBR — composição de efeitos |

### Seção Óptica (16 cards opt_*)

| Card | Equação aplicada | Técnica |
|------|------------------|---------|
| opt_benjamim | $\cos 3\theta$ · textura | PBR — 3 pétalas |
| opt_joaquim | $\cos 2\theta$ · textura | PBR — 2 pétalas |
| opt_yasmin | $\cos 5\theta$ · textura | PBR — 5 pétalas |
| opt_dafny | $\cos 4\theta$ · textura | PBR — 4 pétalas |
| opt_isabelle | $\cos 6\theta$ · textura | PBR — 6 pétalas |
| opt_pegaso | $\cos 2\theta$ · textura | PBR — 2 pétalas |
| opt_merlin | $\cos 1\theta$ · textura | PBR — 1 pétala |
| opt_fourier | $\cos 8\theta$ · textura | PBR — 8 pétalas |
| opt_dark_pontryagin | $\cos 3\theta$ · textura | PBR — 3 pétalas |
| opt_rainha_tiffany | $\cos 6\theta$ · textura | PBR — 6 pétalas |
| opt_rei_cifra | $\cos 1\theta$ · textura | PBR — 1 pétala |
| opt_regente_cristalino | $\cos 6\theta$ · textura | PBR — 6 pétalas |
| opt_ada | $\cos 2\theta$ · textura | PBR — 2 pétalas |
| opt_penny | $\cos 5\theta$ · textura | PBR — 5 pétalas |
| opt_alonzo | $\cos 3\theta$ · textura | PBR — 3 pétalas |
| opt_caelum | $\cos 8\theta$ · textura | PBR — 8 pétalas |

### Cards estáticos (sem canvas — pulados da gravação)

| Card | Equação | Técnica |
|------|---------|---------|
| coracao_revela | $h = g \circ f^{-1}$ | Homeomorfismo esfera→coração |
| captura | Objeto → poinsética → 2ª garrafa → reconstrói | Captura poinsética |
| cifra_ouro | $\sigma_1 = \varphi$ | Crescimento áureo |
| armadura_universal | $\log(ab) = \log a + \log b$ | Logaritmo — produto→soma |
| garrafa_koch | Perímetro $\infty$, área finita | Fractal — Koch |
| ferramenta | Resíduo 0 (duas metades) | Adjuntão $\delta \dashv \varepsilon$ |
| ferramenta_meia | Resíduo $\neq 0$ (falta um lado) | Adjuntão incompleta |
| broca_caelum | Corpo evolutivo $\otimes$ espiral áurea | Broca — twist 3D |
| pulso | $|\omega|=1 \bowtie |\omega|<1$ | Motor — batimento |
| clock | $|w|=1 \bowtie |w|<1$ | Motor — fase |

## Legenda

- **$\varphi$**: proporção áurea $(1+\sqrt{5})/2$
- **$\sigma_m$**: ponto fixo do motor
- **$\oplus$/$\otimes$**: soma/produto do corpo
- **$\chi$**: carácter da dualidade
- **$\mathbb{Z}/n$**: grupo cíclico de ordem $n$
- **S¹**: círculo unitário
- **PBR**: physically based rendering
- **SDF**: signed distance field
- **BRDF**: bidirectional reflectance distribution function