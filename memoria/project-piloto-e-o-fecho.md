---
name: project-piloto-e-o-fecho
description: O manual do piloto, os plugues de dentro (bash e assembly), e o contrato que não morreu — mudou de natureza: liquida-se e chama agentes
metadata:
  type: project
---

**01/08/2026, madrugada.** O pedido era "documentação detalhada pro piloto" e cresceu durante a
sessão inteira, com o Aarão a acrescentar peça a peça. O que ficou não é só documentação: **uma
declaração do sistema morreu.**

## O que se construiu

| ficheiro | o que é |
|---|---|
| `PILOTO.md` | o manual, 12 secções + cartão de bolso |
| `tools/erg.c` | **o plugue de assembly** — monta, corre, desmonta a ISA ERG-64 (19 unidades) |
| `tools/plugue.sh` | **o plugue de bash** — os verbos, e todo verbo chama o `erg` |
| `banco/apps/` | quatro apps, nenhum com mais de 12 linhas |
| `tools/painel.sh` | **o painel**, paralelo ao manual: ele diz o *estado*, o manual diz o *que é* |
| `tools/fecha.c` | **o fecho** — dá quatro números, o corpo diz-se (24 unidades) |
| `tools/polar.c` | as duas formas, e a dualidade entre elas (10 unidades) |
| `tools/smartcontract.c` | **o contrato que se liquida e chama agentes** (11 unidades) |

**232 medidores, 230 verdes, 0 falhas.** A túnica ficou ligada no hook (ver
[[project-memoria-versionada]]) com o lado de ESCREVER, que faltava.

## A correção, e a CORREÇÃO DA CORREÇÃO

Ele corrigiu-me **duas vezes**, e a segunda desfez metade da primeira.

> *"não há necessidade de contrato, fecha quando o corpo completa; qualquer representação finita
> fornecida é meia dualidade — o lado branco fecha quando fornece a régua, o lado dual é o negro."*

E ele tinha razão contra o que eu tinha escrito. **O `contrato.h` já sabia**, num comentário:
*"o cliente pode declarar a RÉGUA em vez da dualidade, e o sistema deriva a outra"*. **Eu parei
aí.** Levar isso ao fim apaga o contrato inteiro — a régua não é a QUARTA coisa a declarar, é a
ÚNICA, e as outras três saem dela.

```
o lado BRANCO      alguns termos — 4 bastam (n+2 para grau 2)
a RÉGUA            (B,C), por Cramer, exata em inteiros
o lado NEGRO       ν(a,b) = (a + B·b, −b) — forçado, não escolhido
FECHOU             quando a reversão volta com resíduo 0
```

**E ele recusa:** uma sequência qualquer não fecha, e *um corpo que não fecha não passa a fechar
por declaração*. Com 3 termos NENHUM corpo sai — o mínimo é mínimo.

**É o padrão de [[feedback-verdadeiro-e-parcial]]:** eu tinha o resultado certo (a régua dá o dual)
e parei de perguntar. A pergunta que faltava era *"então para que servem as outras três cláusulas?"*

**E EU FUI LONGE DEMAIS.** Concluí *"não há contrato"* e ia apagá-lo dos documentos. Ele travou:

> *"vale revisão geral nos documentos — pôr uma explicação de como o contrato SE LIQUIDA. Pode ser
> isso: pronto, smart contracts. É um contrato inteligente, chama agentes."*

**O contrato não desaparece: muda de natureza.** O que morre é a ASSINATURA. E as três peças já
estavam no repo, separadas há dias — `contrato.c` verifica sem julgar, `liquida.c` dispara a cada
entrada (o tique É entrada), `fecha.c` deriva. Faltava LIGÁ-LAS, e faltava o que as torna
inteligentes: **chamar agentes**, com o despacho pelo Δ e não por uma tabela minha.

*O sinal para mim: quando eu concluo que uma peça do sistema é desnecessária, a hipótese seguinte
tem de ser que ela é NECESSÁRIA NOUTRA FORMA — não que sobra.*

## O confronto com o estado da arte (§S6 do smartcontract.c)

E aqui a regra de [[feedback-assercoes-vazias]] mordeu antes de eu errar: **a tabela literária é
asserção vazia.** Então as asserções são todas sobre ESTE sistema, e a literatura fica em prosa
marcada. As três dificuldades canónicas:

| | a resposta corrente | aqui |
|---|---|---|
| a **paragem** | *gas* — orçamento por fora | sai da ÁLGEBRA: `Z_q²` é finito, fecha em π(q) |
| a **reentrância** | disciplina de escrita | o agente NUNCA vem da entrada |
| o **oráculo** | desloca a confiança | o fecho é interno — **o resto não** |

**E o que NÃO se resolve entrou no paper**: sem consenso distribuído, teste e não prova formal,
grau 2 e não computação geral, e o oráculo continua preciso para dados do mundo. *Dizer o contrário
seria vender o que não temos.*

## Os achados que não foram postos à mão

- **A cobertura fecha, e a maior órbita é π(q).** As órbitas de ×σ partem `Z_q²` (cobrem e não se
  sobrepõem) em q = 3,4,5,7,11,12 — e a maior órbita é **exatamente o período de Pisano**. Saiu da
  contagem, não da fórmula. Liga a [[project-hopfield-torres]] e ao `toolkit_llm.c`, que já tinha
  medido π(12)=24.
- **Juros compostos RECUSAM, e a recusa é a resposta certa.** Numa PG o determinante `x₁² − x₀x₂`
  anula — é ordem 1, não 2. *Meia dualidade literal:* há o lado multiplicativo e não há o outro,
  porque C=0 ⟹ N(σ)=0, e o que tem norma nula não inverte.
- **O espelho tem outra régua e o MESMO Δ** — a torre dual é a mesma torre lida do outro lado.
- **A polar tem RAMO e a algébrica não** (25 dos 49 pontos fora do cone, no ouro), e **carrega
  dois bits de sinal** que o círculo não precisa, porque cosh nunca é negativo.

## A dualidade das formas, que o Aarão fechou

> *"forma algébrica escrita é produto DIRETO, forma polar é produto CRUZADO; são duais, um direto
> e um cruzado para cada lado da torre."*

```
⟨x,y⟩ = ρρ·cos_Δ(Δθ)        o DIRETO vê o cosseno    — MEDE    — sob ν FICA
x ∧ y = ρρ·sin_Δ(Δθ)/|τ|    o CRUZADO vê o seno      — ORDENA  — sob ν TROCA de sinal
```

E o regime é o **sinal do Δ** e nada mais: centrando em τ = σ − B/2 sai τ² = Δ/4, e cos/cosh/reta
são a **mesma série**. É [[project-hopfield-torres]] no plano das formas.

## O que impede o medidor de desaparecer

O `erg.c` §E1 **abre o `sql.c`, extrai o enum e compara nome a nome, número a número.** Não é
paranoia: é a única prova de que duas transcrições da mesma ISA são a mesma. Ver
[[feedback-dois-caminhos]].

E os três novos ficheiros foram **citados no catálogo antes de correr a bateria** — senão saem
dela em silêncio. Confirmei que o `erg.c` não estava citado e teria desaparecido.

## O bug que estava lá antes de tudo

**O `catalogo.tex` NÃO COMPILAVA.** Um `\N` sem macro, na linha 855, anterior a esta sessão
inteira. Achei-o só porque compilei para verificar a MINHA inserção — e o primeiro reflexo certo
foi `git stash` para ver se o erro era meu. Não era. Corrigido: **88 páginas**.
Ver [[feedback-o-disco-limpo]].
