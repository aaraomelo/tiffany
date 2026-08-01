---
name: project-checkpoint-2026-08-01-maquinas
description: "Checkpoint 01/08/2026 (as máquinas) — o torque É o cruzado, a rede diferencial, e a cadeia de três elos de Shockley ao braço"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T15:27:11.800Z
---

# Checkpoint 01/08/2026 (as máquinas) — tiffany

Continuação do [[project-checkpoint-2026-08-01-tarde]]. Dois commits, `a039b27` e `0deba5b`.
**Bateria: 195 medidores, 193 verdes, 0 falhas.** `teoria.tex` 95 páginas. Corpus 394 pares.

## O achado: o torque É o produto cruzado

**`tools/motor.c`** — da **monografia do Aarão**: *Controle Direto de Torque do Motor de Indução
Trifásico*, UFRR / Centro de Ciências e Tecnologia, **2017**, orientadora Profa. Dra. Susset
Guerra Jiménez, em `hiper/aposentados/teoria/papers/tcc_dtc_2017/DTC.tex`.

E a peça central estava à espera desde o princípio do projeto:

    T_e = (3/2)·P· ψ_s × i_s        O TORQUE É O PRODUTO CRUZADO.

**Não é analogia.** A parte simétrica devolve escalar, não vê a ordem e não gira nada — é a
norma, e no ferro é a **potência reativa**. A antissimétrica devolve vetor, É a ordem, e roda o
eixo: trocar `ψ` com `i` **inverte o sentido de rotação**. *Um motor é uma máquina de extrair a
metade antissimétrica.*

Também medido: Clarke leva 3 fases pulsantes em **1 vetor girante**; os **8 vetores do inversor
SÃO GF(2)³** (6 no hexágono, 2 nulos — as palavras constantes `(0,0,0)` e `(1,1,1)`); a **tabela
de Takahashi sai da INTERSECÇÃO** de duas histereses, única em cada caso; o **multinível** alivia
a amputação (30° de pior erro com 2 níveis, nunca piora ao subir).

## A rede: J e Jᵀ são o par adjunto

**`tools/robo.c`** — o corpo de corpos posto a trabalhar. O **jacobiano É o corpo diferencial**,
porque `J` é a derivada da cinemática. E fecha com o §B12:

    ẋ = J·q̇          a velocidade SOBE       (a torre branca)
    τ = Jᵀ·F         a força DESCE           (a torre negra)
    ⟨F,ẋ⟩ = ⟨τ,q̇⟩    a POTÊNCIA é a mesma

**A adjunção do §B12 com unidade de watt — chama-se conservação de energia.** E a assimetria
certa continua: `J` perde, `Jᵀ` não é sobrejetiva, cada uma falha onde a outra fecha.

A **singularidade é a degenerescência** (`√det(JJᵀ) = 0` no braço esticado *e* no dobrado — o
`ε²=0`, a raiz dupla). E o corpo de corpos **com preço**: a terceira junta não dá 3 graus, dá
**redundância** — um núcleo de movimentos que não movem a ponta.

## E a generalização: a família ⋆_s (commit `6d0f39c`)

**`tools/dtcn.c`** — e o achado é que a família **É** a fórmula das quatro peças, com um botão no
**cruzado**:

    z ⋆_s w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + s·a×b)

Medido que variar `s` move **apenas** a quarta peça. *Mexer no s é mexer na ORDEM, não na medida.*

E daí a peça nova, a **lei de conservação algébrica**:

    ‖z ⋆_s w‖² + (1−s²)·‖a×b‖² = ‖z‖²·‖w‖²

com `V(s) = (1−s²)·m` o **imposto algébrico**. Sai de Lagrange, e **anula exatamente em s = ±1**
— o Hurwitz, mas agora a classificação **não é uma lista de dimensões: é o conjunto de zeros de
uma função**. Os campos locais `C_â` são fechados, isomorfos a `C`, e lá o imposto é zero — *o
preço está nas fronteiras*.

O controlo generaliza junto: torque **vetorial** (3 componentes em `n=4`, SO(3)), histerese
**esférica** que numa componente **é** o comparador de Takahashi, e bandas que **derivam do
imposto**.

**ACHADO — uma discrepância no paper.** Ele dá compacta e expandida como iguais, mas
`Im(conj(ψ)⋆_s i)` expandido dá o cruzado com **menos** (o conj troca o sinal de ψ e o cruzado é
antissimétrico). Medi as duas: `+2,7` contra `−2,7`. **É a expandida que reduz ao clássico** —
o requisito que o próprio paper impõe — e é a que se implementa.

## A cadeia, com três elos de comprimento

    Shockley -> NAND -> ALU -> microcontrolador     (eletrico, amplifica, mcu)
    micro -> tabela DTC -> inversor -> UM motor     (motor.c)
    N motores -> jacobiano -> a ponta               (robo.c)

E a lei que atravessa os três é uma: **o cruzado gira, o direto mede, e o par adjunto conserva.**

## Onde as fontes estão

- `hiper/aposentados/teoria/papers/tcc_dtc_2017/DTC.tex` — a monografia (§ modelo vetorial,
  § inversores, § DTC). **Atenção ao SINAL**: com os dois fluxos vale `T_e = −(3/2)P(Lm/Lσ)·ψs ×
  ψr'`, ou seja `ψr' × ψs`. Errei isso e o controlo empurrou para o lado errado.
- `hiper/aposentados/teoria/papers/paper_H_dtc_hipercomplexo.tex` — o DTC generalizado à família
  `⋆_s`. **MEDIDO** em `tools/dtcn.c` (ver abaixo). Ficam por medir as extensões E6–E12, que são
  o eixo **preditivo** (sequências de ordem `m` de Lopes 2000, `PA_m`/`PG_m`, o Teorema da
  Unificação) — esse é o passo seguinte natural.
- `chess/sandbox/corpo_transistor.tex` — a arquitetura do microcontrolador

## O padrão de erro desta rodada

Além do de sempre (ver [[feedback-assercoes-vazias]]), dois novos e ambos custaram tempo:

1. **Parâmetros que não fecham ENTRE SI.** Pus `dt = 2e-4` (fluxo andava `1,3e-4`/passo, uma
   volta pedia 48 mil passos) e o rotor com `ω = 12 rad/s`, dezenas de vezes mais rápido que o
   estator. A máquina "não girava" — e eu quase culpei a lógica do controlo. **Antes de culpar a
   lógica, conferir se as escalas dos parâmetros são compatíveis.**
2. **A apresentação a mentir, outra vez.** A tabela do §R4 rotulava "regular" a configuração
   `q₂ = π`, que é singular, porque o limiar era absoluto (`1e-9`) e o valor era `1,1e-8`. E a
   asserção passava por eu só verificar o outro caso.
