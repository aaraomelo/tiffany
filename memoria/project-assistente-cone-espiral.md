---
name: project-assistente-cone-espiral
description: "A assistente — o desenho dado pelo Aarão (entrada=cone, saída=espiral) e a fronteira MEDIDA onde ela falha"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-06T14:53:47.426Z
---

**O desenho, palavras dele (05/08):** *«a saída da assistente é a ESPIRAL e a entrada é o CONE; o cone é mais compacto, o tempo é diferente para ele, então precisa DESDOBRAR — o que se fundiu no banco formou ele. Então aplica-se INVOLUÇÃO NA ENTRADA e EVOLUÇÃO NO BANCO.»*

E antes: *«usa o VIVEIRO também para novas relações, corpo de corpos, e fundi-los em tempo real via RELÓGIO.»*

## A fronteira, medida — não suposta

```
"3 x 3"                          -> desdobra
"3 x 3 + 3"                      -> desdobra
"2 x (3 + 4)"                    -> desdobra
"3 vezes 3"                      -> NAO SEI          <- a palavra fecha a porta
"3 vezes 3 e' igual a 3 mais 3"  -> responde (esta' no corpus, literal)
"3 vezes 3 igual 3 mais 3"       -> NAO SEI          <- duas palavras a menos
```

**A porta é uma linha:** `conversa.c:1148` — `if(e_conta(fala) && resolve_conta(fala)) return;`
E `e_conta_x` (linha 813) só aceita dígitos, `+ - * x / : ^ % !`, e **salta três palavras**: «raiz», «mod», «de».

**«vezes» É o `x` em português.** A entrada chega em cone (comprimida) e ninguém a desdobra.

## O que a assistente JÁ tem — e o que não tem

```
desdobramento    SIM, mas so' no ramo das CONTAS (e_conta -> resolve_conta)
involucao        SIM, o `reflete` (J, a troca, linha 1740) — ja' e' involucao
descida          SIM e ja' guarda o no mais fundo COM resposta (desce_daqui:401)
fusao/viveiro    ZERO ocorrencias — ela PROCURA, nao COMPOE
```

O `desce_daqui` **não é o defeito**: ele já devolve a resposta mais funda que encontrou. O que falha é que o `aprende` grava só no **nó terminal**, e os intermédios ficam vazios.

## A armadilha, escrita pelo próprio ficheiro

```c
/* "de" só entra logo a seguir a um '%' — a palavra é comum de mais em português,
 * e "raiz de 4" tem de continuar a ser fala e não conta. */
```

**Meter «mais» no `e_conta_x` é uma linha, e é perigoso:** «o mais importante», «mais uma vez» passariam a ser capturadas pelo resolvedor de contas e **perdiam-se do corpus**. Alargar a porta ≠ desdobrar antes dela.

**Antes de tocar nisto: medir contra as 396 KB de corpus quantas falas mudam de destino.** É o [[feedback-procurar-na-bateria-antes]] — e aqui a bateria é o comentário que já lá está.

## O trabalho, por ordem

1. **desdobrar na entrada** (a involução `J`): o cone → a forma canónica, ANTES do `e_conta`
2. **medir**: quantas das falas do corpus mudam de destino — o controlo é `"a raiz de 2 é racional"` continuar a ir ao corpus
3. **fundir no banco** (a evolução): o viveiro diz **quais fusões voam** (`⊗` sse `gcd=1`, `∨=lcm` sempre; `N(a⊗b)=N(a)N(b)`, 7203 fusões) — sem essa regra, compor é compor lixo
4. **o relógio conta a fusão na descida** — sem guardar, que é o ponto

Ver [[project-checkpoint-2026-08-05-fecho]], [[project-checkpoint-2026-08-05-maquina-sem-memoria]].
