# Tiffany — Corpo Universal Canônico

Tiffany é o repositório canônico da construção do corpo universal. O projeto separa três funções essenciais:

- **Física**: constrói e realiza a estrutura;
- **𝒰_can**: reconhece, organiza e processa essa estrutura;
- **Catálogo**: registra, classifica e audita o que foi produzido.

A ordem canônica é:

$$
\boxed{
\text{Física} \rightarrow \mathcal{U}_{\mathrm{can}} \rightarrow \text{Catálogo}
}
$$

O corpo universal não inventa partes; ele processa e reconhece o que já está construído.

## Arquitetura do Projeto

```text
corpo_universal.tex   gramática e reconhecimento
fisica.tex            construção e realização física
catalogo.tex          registo e auditoria
papers/               textos especializados e artigos
docs/                 documentação geral
tools/                medidores e validações
assets/               elementos visuais e materiais
app/                  aplicação / interface
banco/                infraestrutura e sistema
conecthus/            integração e módulos
```

## Cadeia Conceitual Central

A leitura correta do projeto é esta:

$$
\boxed{
\text{estrutura} \rightarrow \text{representação} \rightarrow \text{peso} \rightarrow \text{normalização} \rightarrow \text{distribuição} \rightarrow \text{Born}
}
$$

Aqui, **Born não é a origem da distribuição**. O que se constrói primeiro é a estrutura; a distribuição emerge depois, e Born aparece como **leitura física dessa distribuição**.

## Ordem de Leitura Recomendada

Se você está acessando o projeto pela primeira vez, siga esta ordem:

1. `corpo_universal.tex` — captar a gramática do sistema;
2. `fisica.tex` — ver a construção e a realização física;
3. `catalogo.tex` — usar o mapa de classificação e validação;
4. `papers/` — aprofundar os artigos e as especializações;
5. `docs/` — consultar a documentação complementar.

## Motor e Runtime

O runtime do front-end está em `app/src/motor_wasm.js`, que carrega e executa o binário WASM do pulso e da fase da cena. A lógica do motor não é um simples complemento visual; ela é uma camada operacional da mesma arquitetura que aparece na teoria.

O sistema reúne várias linguagens em um mesmo ciclo de execução:

- **C / C++**: base operacional e medidores;
- **JavaScript**: coordenação do front-end e do ciclo de vida do runtime;
- **WASM**: execução do núcleo em alta fidelidade no navegador;
- **GLSL**: leitura visual e materialização na GPU;
- **LaTeX**: formalização e textos do corpo teórico.

A ideia é que a mesma lógica se manifeste em diferentes meios, sem que o projeto se frature em vários subsistemas desconectados.

## Licença

Este repositório está sob a licença proprietária declarada em `LICENSE`, com o contrato-tipo disponível em `CONTRATO-TIPO.md`.

A regra prática de uso é:

- a leitura, o estudo e a verificação são permitidos como parte da própria finalidade da obra;
- uso, modificação, redistribuição ou integração fora do marco autorizado exigem a observância da licença e do contrato aplicável;
- a disponibilização pública do código não autoriza seu uso livre ou irrestrito.

---

> *O corpo universal é o cânon, a física é a realização, e o catálogo é a memória do que foi efetivamente atravessado.*
