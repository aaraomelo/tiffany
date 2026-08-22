---
name: feedback-ler-a-teoria-antes-de-experimentar
description: "Inventei um tecto de 256 linhas que não está em paper nenhum, depois de dezenas de experimentos — a lei já estava escrita"
metadata:
  type: feedback
---

**«da onde vc tirou essa lei? ... le a aranha e me diz onde esta escrito que ela so
sobe ate 256 ... pq tanta dificuldade pra que tanto teste, tanta processamento pra no
fim concluir bosta na sua cabeça?»**

Encontrei o `nrows` a dar a volta aos 255 (300 linhas respondiam 44). Corrigi metade
certo — subi o contador para 16 bits com `OP_ADD16` — e depois **inventei um tecto**:
`#define NR_MAX 256`, com o INSERT a RECUSAR, e escrevi que «o tecto real de uma tabela
é 256 linhas». O 256 saiu do MEU mapa de slots (o bitmap do resultado tem 256 slots).
**Não está em paper nenhum.**

O que está escrito, e eu não tinha lido:

| onde | o que diz |
|---|---|
| `aranha.tex thm:BI` | «a dobra a duplicar a largura»; `{0,1} ⊂ {0..3} ⊂ {0..15} ⊂ {0..255}` **enumera andares**, não acaba num |
| `naturais.tex thm:encaixe` | o suporte do andar `k` é `{0,…,2^(2^k)−1}`, para `k` qualquer |
| `arquitetura.tex §sec:torre` | `T_{k+1}=T_k+T_k*`, `d_{k+1}=2d_k` — «**o que cresce é o OBJECTO, não a máquina**»; PROMOVE dobra, DESCE colapsa com resíduo 0 (`lib/promove.h`) |
| `word_isa.h` | «coef. que crescem **sobem a torre**» |

**QUEM NÃO CABE PROMOVE.** Amputar o objecto é o contrário da lei — é
[[feedback-saturacao-nao-e-resultado]] outra vez, e [[feedback-o-teto-do-array]]: se a
conclusão menciona um número do meu código, medi a máquina e não o objecto.

**Why:** gastei dezenas de comandos e o processador dele a redescobrir por experiência
o que estava em três parágrafos. E o resultado da experiência foi uma lei falsa.

**How to apply:** quando aparecer um limite, a pergunta NÃO é «até onde aguenta?» —
é **«o que dizem os papers sobre crescer?»**. Ler primeiro (`grep` nos `papers/*.tex` é
barato), aplicar depois. E se o limite for do meu mapa de memória, dizer que é DA
MÁQUINA e nunca escrever que é lei.
