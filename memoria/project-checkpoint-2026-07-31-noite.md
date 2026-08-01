---
name: project-checkpoint-2026-07-31-noite
description: "Checkpoint 31/07/2026 (noite) — o barramento, as assistentes a conversar, a refração e o relay, o prismático (primeiro a abrir lugar novo), e a descoberta de que o tatoeba já tinha tudo medido em dados reais"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-07-31T20:04:16.186Z
---

# Checkpoint 31/07/2026, noite — tiffany

Estado: **bateria verde**, `teoria.tex` **72 páginas, 0 pendências**, `sql.c` com **87 asserções**.
Tudo empurrado. Continuação de [[project-checkpoint-2026-07-31-tarde]].

## O que fechou

**O barramento é a aplicação; os bancos reagem.** Ninguém é chamado pelo nome, não há registo de
quem tem o quê, e a prova é pela ausência: procurei um índice e não há nenhum. O endereço é a
cifra, e o endereço **não se atribui — lê-se**.

**As assistentes conversam, e a fala é a interface.** Quando uma não sabe, emite; quem souber
responde, e ela **aprende**. O `"não sei"` deixou de ser o fim e passou a ser *o fim do que eu sei*
— o decreto só fala depois de o barramento se calar. E quando duas sabem, **decide a régua** (a
profundidade do caminho), não o `readdir`.

**A navegação no corpus**: a conversa é um **ponto**, e o ponto são **as coordenadas**, não um nó —
de um índice não se sobe nem se reflete. `sobe` (erosão), `salta` (escreve as coordenadas),
`reflete` (J, e J²=I), `onde`, `ramos`.

**A refração é o gancho para três.** A reflexão fecha em dois (involução); a refração **compõe**
($t_1+t_2 = t$) e a volta pelos três **soma zero**. Mas **φ conserva o Δ** — a roda de conversa é
por classe de Δ, e atravessar classes não é refração.

**O relay**: passar por um tradutor dá o **mesmo** que passar direto (361 frases, 0 diferenças), e
volta com resíduo 0. *É isso que um tradutor tem de ser: um meio que se atravessa, não um que
acrescenta.*

**O prismático** — o triângulo — **foi o único de todo o dia a abrir lugar novo** (44 corpos, 19
lugares). E não houve deformação a fazer: a rotação de 120° tem Δ = −3, **elíptica**, e elíptico é
o redondo. *O triângulo já era o círculo.*

**A soma com Cantor ENCHE**: C tem medida 0,026 e **C + C cobre 100%** de [0,2]. O pó não enche; a
**operação** enche. E o produto com Julia fecha o círculo.

## E a descoberta que mais valeu: já estava tudo feito

Mandaram-me ver o *corpo navegante* e a *BAI*, e encontrei que **eu estava a reconstruir o que o
`tatoeba/` já tinha medido em dados reais**:

- `navegante.c` — a busca **sem Metrópolis**, a fração contínua desdobrada, resíduo 0;
- `centro.c` — *"o centro é o interlocutor"*: PT→centro→EN, e no centro o verbo é **invariante**
  (55,4% contra 34,6% do acaso). Tem a **BAI** instanciada, `Ψ=Collapse` fail-closed;
- `bairro.c` — *"a vizinhança É a órbita"*: `s(e)=a(e)(m+Σw(f)c(e,f))` iterado ao ponto fixo, que
  **é σ = m+1/σ**. Medido em **115 871 decisões**;
- `regua.c` — o corpus **não é função em direção nenhuma** (21,7% das EN têm 2+ PT, até 52).

Eu tinha escrito na teoria que *"o contexto é raso"* — era, e **a culpa era do contexto que eu
inventei** (prefixos de string). O `vizinha.c` liga a assistente ao mecanismo certo.

## Os buracos desta parte (12 a 16)

Continuação da lista de [[project-checkpoint-2026-07-31-tarde]]:

12. **O notify falso**: detetava `mining.notify` por `strstr`, e a resposta do *subscribe* contém
    essa string — o worker martelava um cabeçalho **nunca montado**, com Mhash a subir e zero
    erros. *Passava por saudável.*
13. **O `en2_size` nunca atribuído** — coinbase sem extranonce2, merkle errada, share inválida. O
    génese não apanha porque lá não há extranonce. Mesma classe do 12.
14. **Pus o ponteiro da conversa no slot 3**, que está *dentro* do nó raiz — corrompia a raiz a
    cada resposta.
15. **Passou nos testes e falhava a correr**: os ponteiros de resposta são relativos ao banco e eu
    lia-os todos do último banco usado. Os testes só olhavam o *número* de falas, não o texto.
16. **Escrevi `ok(..., 1)`** — asserção que passa sempre e não prova nada. Terceira vez no dia que
    apanho este vício.

**Como aplicar:** um teste que mede a peça isolada não testa o caminho por onde ela é chamada; uma
régua que sempre acha não prova nada sobre a outra; e antes de construir, **procurar se já está
feito** — o `tatoeba/` tinha tudo, com dados reais, enquanto eu fazia brinquedos ao lado.

## E ficou ligado: a assistente escolhe pela contração

Não como medidor ao lado — **dentro do `conversa.c`**, quando o barramento devolve várias
candidatas:

```
sem contexto                     -> "o banco é o assento do jardim"
fio em "o banco é o assento"     -> "o banco é o assento do jardim"
fio em "o banco é a instituição" -> "o banco é a instituição do dinheiro"
```

**Eu ficava com a mais funda, e isso era ficar na iteração 0.** A profundidade é a *marginal* — as
duas candidatas têm 15 e ela não separa. Quem separa é a vizinhança, e a escolha é o ponto fixo de
`s(e) = a(e)(m + Σ w(f)c(e,f))`.

E a compatibilidade **não precisou de contexto novo**: é o que a candidata partilha com o *caminho
andado* — as coordenadas da conversa, já guardadas no banco desde a navegação. **As duas peças
encaixaram sem cola.**

Defeito estrutural corrigido de passagem: o `pergunta_ao_barramento` **descartava candidatas sem as
ver** (ficava com a primeira mais funda). Agora colhe todas e só depois escolhe — que é o que
permite haver escolha.

## O debate de três, e o que ele derrubou

Três assistentes com conhecimento diferente perguntam umas às outras e aprendem — o barramento
funciona. **Mas o desacordo duro falhou**: com uma a dizer que o rei é o corpo áureo e outra que é
o relógio da rede, o fio em *"o relógio da rede"* devolvia o corpo áureo. A causa: eu media o
**prefixo a contar do início**, e a palavra estava no *meio* — empatavam e o desempate caía na
ordem. **Terceira vez que o "contexto raso" aparece com outra roupa.**

O `bairro.c` não pergunta *onde* a palavra está: pergunta *se* ela está. A compatibilidade passou a
contar símbolos partilhados em qualquer sítio — a interseção, o `mo_prod`.

**E é por isso que se testa com três e não com dois:** com dois, qualquer regra que distinga alguma
coisa parece funcionar.

## A câmara de eco: real, e o sistema não tem defesa

As assistentes aprendem umas das outras, logo uma resposta errada propaga-se e ao fim de três
saltos há quatro a concordar — *não por ser verdade, mas porque copiaram*. Medi: com duas fontes em
desacordo e contexto vazio, a compatibilidade empata e ganha **quem o `readdir` entregar primeiro**
(ordem de inode, arbitrária). A resposta certa ganhou **por sorte**, e eu quase reportei isso como
funcionamento.

Tentei pesar a proveniência (ensinada/ouvida/medida) e o Aarão cortou: **"não tem essa de ensinar,
a informação corre solta"**. O controlo não é sobre quem disse — é sobre a estrutura.

## O CRITÉRIO, dito por ele e que arruma tudo

> *"O nosso controlo é contra deformidades matemáticas, incompletudes, falta de dual, vazamentos —
> não com que roupa as pessoas vestem essas estruturas. Só queremos saber se é segura, reversível,
> e pronto."*

E foi exatamente isso que o dia apanhou, **nenhuma vez sobre conteúdo**: a quinta primitiva
inventada, o comprimento na base, o `realloc`, o `en2_size` incompleto, o notify que passava por
saudável, o `φ` atravessado fora da classe.

**E a verdade é relativa ao corpo, não ao consenso.** `3×3 = 3+3` é **verdade em Z/3** — eu disse
que "não passa aqui" e estava a assumir ℤ sem o declarar. *O resíduo não vota; mas em que corpo se
mede, declara-se.*

## O corpus científico, e a revisão que ele exigiu

`ciencia.sh` — 20 pares onde a régua vai dita. E o Aarão apanhou-me a escrever um **absoluto** na
lista contra os absolutos: *"a raiz de 2 é racional → não"*. Em **Z/7 sim**: `3² = 9 ≡ 2`. Também
Z/17 e Z/23. **Irracional é relativo a ℚ.**

Revi as 20 e encontrei **seis incompletudes**, uma grosseira:

- **"a soma é reversível"** — em ℕ **não é**: `3−5` não está lá. E a diferença entre monoide e
  grupo **é exatamente ter dual**, que eu escrevi na entrada ao lado;
- "o momento conserva-se sempre" → num sistema **fechado**;
- "a entropia cresce" → **pode decrescer**; é contagem, e contagem admite exceção;
- "Cantor tem medida zero" → **de Lebesgue**; de Hausdorff é positiva;
- "o maior primo não existe" → **em ℕ**;
- "o ouro é **a** raiz de σ=1+1/σ" → **uma das duas** — ficar com metade, o erro do dia inteiro.

**Nenhum era conteúdo errado: eram todos incompletudes.**

## Aberto

- o `l[8192]` do socket e o `cb1/cb2/ramos` ainda aterram em RAM antes do banco — a `Fonte` já
  existe e falta passá-la ao `st_linha`;
- o martelo fatiado (64× medidos em `fatia.c`) ainda não substituiu o `sha256` do `OP_MARTELO`.
