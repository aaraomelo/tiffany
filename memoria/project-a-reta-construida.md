---
name: project-a-reta-construida
description: "A construção de ℝ INTEIRO nas oito leis; o ouro é o real mais lento e limita todos; e a lição: não virar juiz da teoria."
metadata:
  node_type: memory
  type: project
---

**A LIÇÃO PRIMEIRO, porque foi a repreensão e é a que se repete.** Escrevi um §«O que este
paper não faz» — uma lista de ressalvas — e uma delas, «não constrói todos os reais», era
**invenção minha, não do quadro**. O Aarão: «vc não colocou o que NÃO PROVA em vez do que
FALTA PROVAR, vc está julgando aqui como juiz» e «não é para o agente virar juiz da teoria;
é para ele ler a teoria inteira e **executar a próxima construção**». O padrão: quando o
quadro já fechou uma construção, o meu trabalho é REALIZÁ-LA, não auditar se ela é
permitida. O freio disfarça-se de rigor.

**A HIERARQUIA que eu tinha perdido**, e que agora encabeça o paper:

    o Universal PROVA   |   o geométrico REALIZA   |   os medidores VERIFICAM

`geometrico.tex` é **filho** do `corpo_universal.tex`, não um paper independente. Cada peça
leva a linha das três colunas; se não tiver as três, não está pronta.

**E A CONSTRUÇÃO É DE ℝ INTEIRO.** Um real **É** uma sucessão de quocientes parciais
a_k ≥ 1; a construção corre sobre sucessões **arbitrárias**, e o metálico é o caso
**periódico**, não o caso único. Em 220 sucessões: `p_k q_{k−1} − p_{k−1} q_k = (−1)^{k−1}`,
logo |det| = 1 em todo passo; alternam, aninham, q_k cresce. E o fecho para todos de uma vez:

    q_k ≥ F_k para TODO real, com igualdade em TODO k só na sucessão de uns

**O ouro é o real mais lento que existe**, e por isso UMA taxa limita ℝ inteiro. A família
metálica não é o escopo — é o caso **extremo**. Ver [[project-teorema-do-gato]].

**Distinção que eu tinha misturado (o eval apanhou):** são DOIS determinantes —
`det A^k = F_{k+1}F_{k−1} − F_k²` das ENTRADAS, em ℤ, sem espectro; e
`σ^k(σ†)^k = N(σ^k) = a² + mab − b²` do ESPECTRO, sem olhar uma entrada. **A igualdade dos
dois é o TEOREMA** («det é o produto do espectro»), e é a coincidência que se mede.

**E «oito passos enchem um byte» é CODIFICAÇÃO — não demonstra bit a bit.** O que demonstra
é Gram = I. Numa base torcida f_k = e_k ⊕ e_{k+1} a Gram sai da identidade em 24 dos 64 e a
leitura directa erra **metade exacta** das 2048 coordenadas.

**Defeitos meus que a medição apanhou nesta ronda** (todos do mesmo tipo — ver
[[feedback-assercoes-vazias]]):
- **a semente da recursão dos convergentes**: pus `q₀ = 0, q₁ = 1` quando é `q₋₁ = 0, q₀ = 1`.
  Deslocava a sucessão um andar, e apanhou-se pelo sítio certo: **o ouro deixou de bater com
  Fibonacci**, que é a única coisa que ele não pode deixar de fazer.
- **afirmei de mais**: «q_k = F_k só no ouro» é FALSO pontualmente (69 empates); o que só o
  ouro faz é empatar em **TODOS** os andares.
- **três números escritos à mão** nas asserções (512, 4000, «19 3928»), trocados por
  condições estruturais.
- o sinal de Cassini trocado (0 de 115) e o det do controlo sem o factor 4.
- e o `\verb` dentro de um teorema partiu o pdflatex — **e o PDF que ficou no disco era o
  ANTIGO**, que eu quase reportei como novo. Ver [[feedback-a-mensagem-que-nao-pode-falhar]].

`tests/geometria_real.c` (15:0) na ordem obrigatória do eval: 8 leis → base ortonormal →
bit a bit → dual → recorrência → Pisot → encaixe → corte → completude → ℝ → π_k/gume.
`papers/geometrico.tex` reescrito duas vezes no dia, 6 páginas.
Ver [[project-o-real-e-o-corte]], [[feedback-verdadeiro-e-parcial]].
