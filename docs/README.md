# Tiffany — Corpo Universal Canônico

Tiffany é o repositório canônico da construção do **Corpo Universal**.  
O projeto não inventa estrutura: processa, reconhece e audita o que já foi construído.

A ordem canônica é:

```text
Física  →  𝒰_can  →  Catálogo
```

\[
\boxed{\mathcal{U}_{\mathrm{can}} := \text{a realização canônica de }\mathcal{U}}
\]

| Camada | Papel | Documento principal |
|--------|-------|---------------------|
| **Física** | Constrói e realiza a estrutura | `fisica.tex` |
| **𝒰_can** | Reconhece, organiza e processa | `corpo_universal.tex` |
| **Catálogo** | Registra, classifica e audita | `catalogo.tex` |

```text
                 CONSTRÓI
                    │
                 Física
                    │
                    ▼
              𝒰_can / Corpo
                    │
          RECONHECE / PROCESSA
                    │
                    ▼
                Catálogo
                    │
                 AUDITA
```

Tiffany não é “mais uma teoria”. É a infraestrutura que preserva a separação entre o que foi construído, o que foi reconhecido e o que foi medido.

---

## Ideia central

O Corpo Universal \(\mathcal{U}\) é o intermediário operacional entre a Física e o Catálogo:

\[
\mathcal{U} = (X, X^{*}, \mathsf{Mor}, \mathsf{Aut})
\]

- \(X\) é o espaço de largura oito (base \(\{e_k = 2^k\}_{k=0}^{7}\));
- as oito leis correspondem às oito realizações obtidas das três dobras (\(8 = 2^3\)); não existe um oitavo gerador axiomático independente;
- o fechamento canônico ocupa a posição terminal sem introduzir uma nova lei — o retorno fecha o ciclo com resíduo zero.

A cadeia conceitual que organiza toda a obra:

```text
estrutura → representação → peso → normalização → distribuição → Born
```

Born não é origem da distribuição: é a **leitura física** da distribuição que emerge depois da estrutura.

---

## Documentos principais

### Núcleo teórico

| Arquivo | Conteúdo |
|---------|----------|
| `fisica.tex` | A Física Araniana. Constrói a estrutura nos catorze lugares onde opera (álgebra, topologia, análise, fractal, geometria, mecânica, gravitação, relatividade, eletromagnetismo, óptica, partículas, termodinâmica, cosmologia, redes). Autocontido. |
| `corpo_universal.tex` | Gramática e reconhecimento. Define \(\mathcal{U}\), as três línguas (álgebra, topologia, análise), a Trindade Canônica, a lei canônica e o motor `INGEST`. |
| `catalogo.tex` | O bestiário dos corpos. Registra cada corpo com suas operações, dual, involução, interface, realização e estado. Auditoria do que foi efetivamente medido. |

### Ponte e arquitetura

| Arquivo | Conteúdo |
|---------|----------|
| `transformada.tex` | Transformada universal — mudança reversível de leitura entre suporte circular, métrica cordal e coordenada linear local. |
| `arquitetura.tex` | Arquitetura computacional: byte level, slots, ISA e API SQL. Do bit à query. |

### Manifestos e mapas (Markdown)

| Arquivo | Conteúdo |
|---------|----------|
| `ESTRUTURA_CORPO.md` | Selo do Corpo Universal. Formaliza a cadeia **Lei → Operação → Corpo → Ordem → Refinamento → Caminho → Limite** e declara o congelamento estrutural. |
| `MAPA_UNIVERSAL.md` | Mapa de dependências. Árvore que liga as 8 Leis até o fechamento da renormalização, com inventário de medições e fronteiras declaradas. |

---

## Cadeia de realização (selo estrutural)

```text
Lei → Operação → Corpo → Ordem → Refinamento → Caminho → Limite
```

- **Completo** significa completude por refinamento operacional da escada — não completude métrica arquimediana clássica de \(\mathbb{R}\).
- Um real é um **caminho** raiz→folha na árvore da torre (fibra 2).
- A álgebra opera e não alcança; a topologia alcança e não opera.

A estrutura está **blindada**: novas migrações de núcleo exigem inventário de ownership e ordem explícita. Todo teorema novo entra pelo contrato \(\mathcal{M}=(R,G,V)\).

---

## Ordem de leitura recomendada

1. `corpo_universal.tex` — captar a gramática do sistema  
2. `fisica.tex` — ver a construção e a realização física  
3. `catalogo.tex` — usar o mapa de classificação e validação  
4. `ESTRUTURA_CORPO.md` + `MAPA_UNIVERSAL.md` — entender o selo e as dependências  
5. `transformada.tex` e `arquitetura.tex` — pontes e realização computacional  

---

## Motor e runtime

A mesma lógica se manifesta em vários meios, sem fraturar o projeto:

- **C / C++** — base operacional e medidores  
- **JavaScript** — coordenação do front-end e ciclo de vida  
- **WASM** — núcleo em alta fidelidade no navegador  
- **GLSL** — leitura visual e materialização na GPU  
- **LaTeX** — formalização e textos do corpo teórico  

O runtime do front-end carrega o binário WASM do pulso e da fase da cena. A lógica do motor é camada operacional da mesma arquitetura que aparece na teoria.

---

## Princípio metodológico

Nada entra sem medidor.  
Nenhum nó da árvore arquitetural existe sem o programa que o atesta.

\[
\boxed{\text{medição operacional} \neq \text{equivalência matemática geral}}
\]

O Universal **não contém toda a matemática**. Possui uma infraestrutura operacional na qual construções específicas foram realizadas e verificadas como elos distintos.  
“Universal” aqui significa infraestrutura comum de realização — não enciclopédia de toda a matemática existente.

---

## Licença

Este repositório está sob licença proprietária.  
Copyright © 2026 Aarão Melo Lopes.

- Leitura, estudo e verificação são permitidos como parte da finalidade da obra.
- Uso, modificação, redistribuição ou integração fora do marco autorizado exigem observância da licença e do contrato aplicável.
- A disponibilização pública do código **não** autoriza uso livre ou irrestrito.

Contacto: aarao.melo.lopes@gmail.com

---

> *O corpo universal é o cânon, a física é a realização, e o catálogo é a memória do que foi efetivamente atravessado.*
