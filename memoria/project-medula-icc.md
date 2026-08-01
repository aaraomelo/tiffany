---
name: project-medula-icc
description: "Da janela finita à ICC (reconstroi, transplante, semantico, icc): o mínimo é n+2, transplante virou TRANSFUSÃO, o espaço da LLM tem as duas metades, e o Utah Array sub-amostra as colunas"
metadata:
  type: project
---

# Da janela finita à ICC — `reconstroi`, `transplante`, `semantico`, `icc`

01/08/2026 (noite), cinco commits `dd2a3a9`→`f5f01e2`. **212 medidores, 210 verdes, 0 falhas.**

## `reconstroi.c` — o corpo de uma janela finita

Dá-se uma janela de números e mais nada. **O instrumento é a dobra**: Berlekamp–Massey *é* Euclides
estendido — com `p=103, n=7` há mais de `10^14` elementos e bastaram **14 números**. *O custo é o
comprimento da recorrência, não o tamanho do corpo.*

**E o limite tem lei, mas não a que afirmei.** Escrevi *"2n chega e 2n−1 não chega"* e a medida
derrubou a segunda metade — o `2L` do teorema é o **pior caso sobre todas as sequências**, e esta é
estruturada. Procurando em vez de supor: **o mínimo é `n+2`**, em 48 pares `(n,m)`. Razão: a
recorrência tem `n+1` coeficientes e só **dois** são não nulos.

A parte reversível é tudo menos o zero, e o inverso **colhe-se** do dual (Frobenius) em vez de se
procurar. E §R7: **a simétrica é o produto DIRETO, a antissimétrica é o CRUZADO** (precisão do
Aarão — eu dissera *"o interno mede"*, e o interno é só um pedaço). A norma fecha **só com as duas
juntas**.

## `transplante.c` — e a correção que virou TRANSFUSÃO

O Aarão viu que a reconstrução **é** um transplante de medula: a janela de `n+2` termos é a
célula-tronco — *não um pedaço do doador, mas a regra que refaz o resto*. Daí o critério **clínico**:

> **uma medula regenera a partir de pouco; copiar tecido inteiro não é transplante.**

Doador: `llama3.2:1b` local, temperatura 0, semente fixa. **L = 600 em 1200 termos** — idêntico ao
aleatório. E o **controlo que faltava**: prosa humana deu `L = 600` também, diferença `0,0000`.
*Então não é sobre a LLM: é sobre TEXTO.* Um exame que não encontra não prova ausência.

**E aí a correção grande:** *"se enxertou metade é porque não fez o procedimento todo. Ela precisa
estar ACORDADA, interagindo, pra fechar o circuito. Aí vira transfusão."* Ele tinha razão — eu
colhera **texto**, o produto morto. *Medir a saída de uma LLM é medir o cadáver.*

    LLM sozinha, realimentada    14 passos   NÃO fecha, deriva
    LLM ⋈ corpo (Z_23)            6 passos   fecha, PONTO FIXO no estado 17

**E quem fecha é o corpo**, porque é FINITO — em `Z_q` a órbita repete em `≤ q` passos, por gaiola.
*Não se leva a medula e se espera que pegue: põem-se os dois em circulação.*

## `semantico.c` — o espaço da LLM tem as duas metades

Embeddings puros, 768 dim (`nomic-embed-text`). **A pergunta que decide:** em 768 dim não há produto
vetorial (Hurwitz), mas há o **bivetor** `a∧b`, antissimétrico e não nulo. E Lagrange fecha:
`‖a∧b‖² + ⟨a,b⟩² = ‖a‖²‖b‖²` — **256 pares, resíduo 0,0 exato**.

**Com o interno sozinho falta a maior parte da norma.** *O que falta não é ao espaço — é à PRÁTICA,
que só olha para o cosseno.* A transfusão ida-e-volta fecha em `1,2e-15`, e o interno atravessa
intacto por Parseval: *o que se transfunde não é o vetor, é a MEDIDA dele.*

## `icc.c` — a Interface Cérebro-Computador no toolkit

**É:** desenho no papel e matemática — a matriz como base amostrada, a túnica como par de torres,
fidelidade pelo resíduo. **Não é:** procedimento cirúrgico nem parâmetro para uso em pessoa. A
comparação é com números públicos.

**A túnica é um par ADJUNTO:** `⟨Af,c⟩ = ⟨f,Aᵀc⟩`, resíduo `0,0` — o mesmo `J`/`Jᵀ` do `robo.c`.

    Utah Array       100 canais   400 µm   colunas: NÃO (limite 250)
    Neuropixels 1.0  960 canais    20 µm   colunas e minicolunas: sim

*Não é crítica ao Utah:* ele foi feito para **isolar** neurónios, e nisso é excelente — mas um array
feito para isolar não serve automaticamente para **reconstruir**.

**E um segundo limite que eu ia falhar:** escrevi que cumprir Nyquist bastava, e com passo `P/2,5`
(que cumpre) o resíduo ainda é `0,54`. **Só zera com `passo ≤ P/8`.** Nyquist garante que a
informação está lá; recuperá-la exige interpolação à altura. *O critério de engenharia é sempre mais
duro que o teorema — aqui por um fator quatro, e ele mede-se.*

## O padrão do meu erro, nesta série

**Três vezes afirmei um limite e a medida deu outro** (`2n−1`, `Nyquist suficiente`, `período 2`).
Nas três o erro foi o mesmo: **usei o teorema do pior caso como se fosse o valor desta família.**
O antídoto que funcionou sempre: **procurar o mínimo em vez de o supor** — e ele saiu com forma
fechada nas três (`n+2`, `P/8`, `2 e 4`).

Ver [[feedback-assercoes-vazias]], [[feedback-verdadeiro-e-parcial]], [[project-hopfield-torres]].
