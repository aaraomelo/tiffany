---
name: feedback-assercoes-vazias
description: "A asserção que passa sem poder falhar — DOZE formas dela, e as três ferramentas que as caçam por máquina"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T17:36:04.756Z
---

# A asserção que passa sem poder falhar

Em 01/08/2026 escrevi **oito** famílias de asserções que davam verde sem medir nada. Não é
distração: é um modo de falhar meu, e tem formas reconhecíveis. Uma asserção vazia é **pior que uma
que falha** — a que falha avisa; a vazia conta-se como prova.

## As oito formas

**1. A constante disfarçada.** `ok("o quadro fecha", 1 == 1)`. E a variante mais difícil de ver:
`int cobertas = 6, semCorolario = 0;` seguido de `ok(..., semCorolario == 0 && cobertas == 6)` —
variáveis que eu próprio fixei duas linhas acima.

**2. A tabela literária.** Uma asserção que compara **strings que eu escrevi** contra strings que eu
escrevi (`strcmp(t[k].g, t[k].dd)` numa tabela de nomes de grupos). Falhou pela grafia
(`"T (o S¹)"` vs `"T"`) — mas o problema real é o oposto: **se eu errar o nome nas duas colunas,
ela passa**.

**3. O número escrito de cabeça.** `ok(..., cobertas == 6)` quando eram 7. Aqui o medidor apanhou —
mas a correção certa não foi trocar 6 por 7: foi medir *a frase que a asserção afirma*
(`coberta ⟺ n par OU n ∈ {1,3,7}`), para não haver número à mão nenhum.

Esta é a que mais se repete. **Três vezes em 01/08:** `"6 cobertas"` (eram 7); `"menos de 0,01%"`
(era 0,0333% — e eu nunca calculei, escrevi o número que *soava* pequeno); `"o salto do raster é
N−1"` (é N, porque o retorno anda N−1 em x **e mais 1 em y**).

**Em 01/08 (noite) foi a QUARTA vez do dia**, no `tikz.c`: *"3 instruções"* (eram 2), *">50
instruções"* (eram 40). E o agravante: **este memory já dizia tudo isto e eu não o consultei antes
de escrever.** É o mesmo que o [[feedback-simulacao-nao-bate]] regista — *escrever não impede,
CONSULTAR antes de escolher é que impede.* O gatilho a treinar: **sempre que a asserção contém um
literal numérico que eu não calculei ali mesmo, parar e perguntar de onde ele veio.**

**O antídoto é sempre o mesmo, e não é escolher melhor o número: é medir a LEI.** Em vez de
`saltoR == 2*N*(N-1)` para um `N` só, correr `N = 3..10` e exigir a fórmula em todos. Em vez de
`variação < 1e-4`, medir que a sensibilidade fica dividida por `(1+A'β)`, que é exato e não pede
limiar. **Uma lei medida em vários pontos não tem onde eu enfiar um palpite; uma constante tem.**

E há uma variante do antídoto que vale guardar, do `tikz.c`: quando a afirmação é *"X não afeta a
contagem"*, **medir a mesma entrada COM e SEM X e exigir igualdade** — aí não há número nenhum na
asserção, e ela mede exatamente o que diz.

**9. A MESMA EXPRESSÃO DOS DOIS LADOS, com um limiar por cima.** Em 17/08 apanhei-a **duas vezes
no mesmo dia**, em ficheiros diferentes, e ela é a mais fácil de escrever sem dar por isso:

- `continua.c §C4`: `fabs(r1 − (−sl)) < 1e-12` com `r1 = (−m+d)/2` e `sl = (m−d)/2`. Então
  `−sl = (d−m)/2`, que é **r1 letra por letra**. Oito metais, oito «sim», e nenhuma entrada podia
  dar outra coisa. O `1e-12` era o que lhe dava cara de medida — é o
  [[feedback-o-limiar-tem-tres-causas]] na sua pior forma: *o limiar a dar cara de medição a uma
  tautologia*.
- `xx.c §X1`: `L a=1, b=1;` calculados por **dois laços idênticos**, e depois `a==b`. E na mesma
  secção, sem disfarce nenhum: **`if(1==1 && 1==1) raiz_1++;`**

**10. A DEFINIÇÃO RELIDA — e é a mais produtiva de todas.** Uma variável é definida por uma
expressão, e uma asserção mais abaixo verifica essa MESMA expressão. Em 17/08 apanhei-a **cinco
vezes em cinco ficheiros**:

```
forca.c        V = Pi*S            depois  razao = V/(Pi*S)          → 1
cosmico.c      Qf = Q - W          depois  W + Qf == Q               → W+Q−W
tikz.c         #define TFIM (N*H)  depois  fabs(N*H - TFIM) < 1e-12  → x−x
spline.c       w = av*s/upem       depois  w12/w10 == 1.2            → 12/10
ttf_corpo.c    buraco = a - b      depois  b < a && buraco > 0       → a mesma condição
```

**Nem o `gume.py` nem o `residuo_zero.py` a apanham**: mutar o operador derruba a asserção na
mesma, e nem todas imprimem resíduo. Por isso nasceu `tools/definicao_relida.py`, que procura a
FORMA — para cada asserção, se a definição de um identificador da condição menciona outro que
também lá está. (A 1.ª versão deu 250 candidatos, quase todos ruído de declarações múltiplas
`int a = 0, b = 0;`; com as vírgulas de topo separadas ficaram 70.)

**O antídoto é sempre o mesmo: a segunda parcela tem de vir de OUTRA VIA.** No `cosmico.c`, calcular
`Q_frio` pela razão das temperaturas em vez de por `Q − W`; no `forca.c`, medir a consequência
(sem cruzado não há imposto) em vez da factorização; no `ttf_corpo.c`, medir a CAUSA (há um contorno
de área negativa) e o contraste (um glifo de sentido único não tem nenhum).

**12. A SOMA DAS PARTES É O TODO.** `ok(..., fecha + fora == ciclos)` — cada ciclo cai num dos dois
contadores, logo a soma é sempre o total. Escrevi-a no `universal.c` e **um gume não mordeu**: foi
ele que a denunciou. O que tem conteúdo é exigir o fecho TOTAL onde nada satura, e contar a
saturação à parte. *Um invariante contabilístico não é uma medida.*

**11. A CONSTANTE QUE PARECE UMA CONTA.** `int nona = (MOD_CAT % MOD_CAT != 0);` — `x % x` é zero
para qualquer x, logo é a constante `false`, e o `&& !nona` na condição nunca podia falhar. Estava
num bloco de onde eu **já tinha tirado três tautologias e escrito porquê**: escapou porque tinha um
operador e um `!=`. *Uma expressão não é uma medida só por ter símbolos.*

**E o gume que RELAXA não é gume.** Duas vezes apontei mutações que afrouxavam a condição (aceitar
zero, verificar menos coordenadas) e concluí «não mordeu». Um gume tem de mutar **o que a asserção
mede**, não a asserção. E quando ele não morde, a causa pode ser a ENTRADA: no `nne.c` o gume ao
`esc` sobreviveu porque `u = {1,0,0}` tinha a terceira coordenada zero — o defeito não vivia ali.

**E o modo mais traiçoeiro dela: SIMPLIFICAR À MÃO NO PONTO.** Em 17/08, a corrigir uma destas no
`dominios.c`, escrevi a «derivada analítica em t=0» já simplificada:

```c
double v0_exacta = -aa*(1.0) + (0.0 + aa*1.0);   /* y'(0) */
```

que é `-a + a`. Zero por ser `x − x`. **Cometi o defeito dentro da correcção do defeito**, e foi a
quarta vez no mesmo dia. A causa: eu fiz a álgebra de cabeça até ao ponto onde ela colapsa, e
escrevi o colapso. O remédio é **escrever a função GERAL e só depois avaliar no ponto** — assim ela
pode estar errada, e uma segunda rota (aqui a diferença finita, corrida FORA do zero) apanha-o. É
[[feedback-a-referencia-escrita-a-mao]] outra vez: se não muda com os dados, é cópia.

**O gatilho:** sempre que uma asserção compara duas quantidades, perguntar *de onde vem cada uma*.
Se as duas descem do mesmo cálculo — mesmo laço, mesma fórmula reescrita, mesma variável negada
duas vezes — não há duas rotas, há uma. É o [[feedback-dois-caminhos]] pelo avesso.

**O antídoto não é escolher outra comparação: é medir a EQUAÇÃO.** No `§C4` passou a ser «−σ e −σ†
ANULAM 1−mx−x²» (zero exacto em ℤ[√D], sem régua). No `§X1` passou a ser «quantos x satisfazem
x^x = x^n?» — varrer, avaliar os dois lados e **contar**: são dois, e um só em n=1. A contagem tem
gume; a identidade repetida não tem.

**E o detector automático apanha-a:** a `tools/gume.py` muta `==` para `!=` e a asserção não cai,
porque `x != x` continua a decidir o mesmo. Ver [[project-gume-automatico]].

**4. O caso degenerado que iguala os dois lados.** A pior, porque parece um teste a sério. Escolhi
`E` em senos e `B` em cossenos para medir o Poynting; são ortogonais, logo `S = 0` **exato**. E com
`S = 0`, as asserções *"preserva"* (`S → S`) e *"inverte"* (`S → −S`) são **a mesma afirmação** —
as duas passaram verdes. Ao pôr `S ≠ 0`, as duas falharam **e revelaram um erro de modelo que eu
nunca teria visto**: usava o produto interno onde o Poynting é o vetorial.

**5. O caso de teste escrito a partir do que eu ESPERO, não do que estou a medir.** Escrevi uma
asserção sobre acentos (`strstr(saiu, "cora\xE7\xE3o")`) sobre um fonte de teste que eu próprio
digitara **sem um único acento** — `"Acentos: coracao, area, tres, voce"`. A asserção nunca podia
passar, mas o dano é o simétrico e pior: se eu tivesse escrito a asserção *também* sem acento, ela
passava verde **sem que um só acento tivesse sido testado**.

**Aconteceu TRÊS vezes no mesmo dia**, sempre com acentos, e a terceira foi já dentro da unidade de
regressão que eu estava a escrever *para o bug dos acentos*. A causa não é o acento: é eu compor o
caso de teste a partir da minha ideia do assunto, em vez de o compor a partir **do defeito
concreto**. O fonte de teste que apanha o bug do cifrão tem de ter um cifrão desirmanado **e uma
palavra acentuada depois dele** — se faltar qualquer um dos dois, não há asserção que o veja.

**Antídoto: escrever o caso de teste a partir do fonte REAL onde o defeito apareceu**, copiando o
trecho que falhou, em vez de inventar um exemplo do mesmo género.

## E a irmã ao contrário: a asserção AMARRADA À RÉGUA

Não é vazia — mede. Mas mede a **régua** em vez do facto, e por isso quebra quando a régua muda,
mesmo com o facto intacto. Ao ligar a curva ao `tex.c`, **três** das minhas asserções caíram, e as
três fizeram bem em cair:

- `largura('W') == 944 && largura('i') == 222` — valores absolutos da tabela. **Um valor absoluto
  amarra a asserção a UMA fonte de medida.** O que eu queria afirmar não era *"o W mede 944"*, era
  *"o W é muito mais largo que o i"* — e a **proporção** sobrevive à troca da régua.
- `curva == tabela` exato, quando o medidor ao lado media com tolerância de 1. Não era discordância
  entre as duas: era o **arredondamento do meu próprio conversor**, que eu negava ao exigir
  exatidão. Medir o arredondamento é melhor que fingir que não existe.
- `distintas >= 20` deu 19. **Baixar para 19 seria escolher a constante outra vez.** O critério que
  ficou é a **razão entre o mais largo e o mais estreito** (5,34) — vale 1 se a régua não medisse
  nada, e isso não se escolhe.

**E o aviso que vem com isto:** inventei **três leis** sobre a mesma tabela de larguras e a medida
derrubou as três (*"a negra é mais larga"*: o `W` é igual nas duas; *"a negra nunca é mais
estreita"*: o `@`; *"nada visível é mais estreito que o espaço"*: o apóstrofo é 190 contra 277).
**Sobre uma tabela publicada não se afirmam leis — mede-se o que ela faz.** Se eu quero uma lei,
ela tem de vir do objeto, não do meu senso do que seria arrumado.

**6. ANOTAR o defeito em vez de o CORRIGIR.** Em 01/08 escrevi `ok("...", 1)` e, ao lado, o
comentário *"a asserção acima não mede nada"* — e deixei as duas coisas no ficheiro. **É pior que
não ver o defeito:** cria a aparência de rigor sobre uma medida vazia, e quem ler a nota assume que
foi tratada. *Ou corrijo na mesma edição, ou não escrevo a nota e deixo o defeito falhar.*

**7. O limiar posto no VALOR EXATO.** `V > 1e-3` para uma grandeza que dá exatamente `1,00e-3`. Um
limiar no valor exato não mede o fenómeno — mede o arredondamento, e falha ou passa por acaso. Se a
afirmação é sobre ORDEM DE GRANDEZA, comparar com a **escala** (*mil vezes o microvolt*), não com o
número.

**8. O ABSURDO NO RELATÓRIO que a asserção não apanha.** No `liga.c` o relatório imprimia
*"reflexão de 159,5%"* — **impossível num material passivo** — e a bateria estava verde, porque
nenhuma asserção olhava para aquele número. Duas causas somadas: o ramo da raiz dava `Re(Z)<0`, e um
`printf` partido em dois ficara **sem os argumentos** (o valor era lixo da pilha).

*Uma asserção verde não certifica o que ela não mede.* **Gatilho: ler os números impressos como se
fossem de outra pessoa, e perguntar se algum é fisicamente impossível** — percentagem acima de 100,
eficiência acima de Carnot, norma negativa, probabilidade fora de [0,1]. E compilar com `-Wformat`,
que apanha o `printf` sem argumentos e que eu não tinha ligado.

## Como aplicar

1. **Antes de commitar uma asserção, perguntar: que valor de entrada a faria falhar?** Se não
   houver nenhum, ela não mede. Se o único for "eu ter escrito outra coisa na linha de cima",
   também não.
2. **O caso de teste tem de ser não-degenerado, e isso mede-se.** Pôr uma asserção explícita sobre
   o pré-requisito (`ok("Σh é NÃO NULO — senão a seguinte mediria o vazio", ...)`) — assim o
   degenerado não pode entrar em silêncio.
3. **Nunca comparar o que eu escrevi com o que eu escrevi.** Tabelas de nomes, listas de
   classificação e constantes de cabeçalho são *citação*, não medida — marcá-las como citadas e pôr
   a asserção sobre um objeto construído.
4. **Quando um valor não bate, perguntar primeiro se o modelo está certo** — vale mesmo quando a
   falha parece um detalhe de sinal. Duas vezes nesta sessão a falha era da operação, não do sinal:
   escalar onde era vetorial, e "inverte as duas" onde cada uma inverte só uma.

## A NONA, de 01/08 de madrugada: **a asserção de unicidade com o crivo FRACO**

Escrevi `ok("há exatamente DUAS involuções que conservam a norma", ...)`. **São dez.** A asserção
caiu — e o defeito não era o número, era o **crivo**: conservar a norma é fraco demais. O crivo
certo era *respeitar o produto*, e com ele há uma só (Galois em grau 2).

**O sinal:** quando afirmo que *só existe um* de alguma coisa, a pergunta não é "quantos contei?"
mas **"o meu filtro é apertado o suficiente para a unicidade ser verdadeira?"**. E a correção ficou
ESCRITA no medidor — `fecha.c` §F2 mede as dez ao lado da uma, para o crivo se ver.

## A DÉCIMA: **a secção inteira definida e nunca chamada**

Acrescentei `secao_S6` ao `smartcontract.c` com quatro asserções, e o `main` nunca a chamou — o meu
`replace` não bateu por causa de espaços. **As quatro asserções não falharam: desapareceram.** A
bateria teria dito "7 unidades, 0 falhas" e estaria certa.

**Quem apanhou foi o `-Wunused-function`.** É o irmão exato de *"um medidor que não compila não
falha, desaparece"* ([[feedback-dois-caminhos]]), agora um nível abaixo: dentro de um medidor que
compila e passa. **Depois de acrescentar uma secção, confirmar que a contagem de asserções SUBIU.**

**E o número de cabeça voltou pela sexta vez** no mesmo dia (contei quatro instruções onde havia
cinco). O antídoto continua a funcionar: **medir a LEI em vários pontos** — `bytes = Σ(1+operando)`
em cinco programas — em vez de acertar melhor no número.

## A DÉCIMA PRIMEIRA, e é a IRMÃ DE TODAS: **a asserção que nunca PASSA**

O Aarão perguntou: *"qual o problema desses dois medidores que não falharam nem passaram?"*

O `ancora.c` e o `homogeneo.c` provam teoremas **negativos** — que a soma-de-palavras não é a
tradução (0,14% e 11,15%). O resultado é verdadeiro. Mas eu afirmava-o **ao contrário**: a asserção
dizia que a rotação *fecha*, e como não fecha ela falhava sempre. E o `bateria.sh` tinha uma
**lista à mão** (`case ancora|homogeneo`) que traduzia a falha em "NEGATIVO, teorema por projeto"
e calava.

**O preço mediu-se:** trunquei o corpus a **três pares** e o `ancora` deu exatamente o mesmo
veredito que com 196 415. O medidor não distinguia *"o teorema negativo confirma-se"* de *"os dados
evaporaram"*.

> **Uma asserção que nunca passa é tão vazia quanto uma que nunca falha.**

É a irmã das 66 `ok(...,1)`, do outro lado do espelho. E o sintoma estava **à vista no relatório**,
em duas colunas que eu lia sem ler: *"2 negativos por projeto"*.

**O conserto — dizer o negativo POSITIVAMENTE:**

```c
ok("a taxa de fecho fica ABAIXO de 5% — o Σ não é a tradução", taxa < 5.0);
ok("o corpus tem pelo menos 10 000 pares — o negativo é sobre DADOS", n >= 10000);
```

Agora passam e **podem** falhar: se a taxa subir (seria um achado) ou se os dados sumirem.

**E a regra geral:** *toda isenção numa bateria é uma lista à mão, e toda lista à mão acaba a
desculpar o que devia medir.* Um medidor com direito a falhar não é medido. A categoria inteira
foi-se — 232 de 232 verdes, zero isenções.

## A DÉCIMA SEGUNDA: **o critério que JÁ É VERDADE por construção**

Pedi ao modelo que a resposta dele tivesse espectro **conjugado** do da frase — e para um sinal
**real** isso já é verdade sempre: `F(N−k) = conj(F(k))`. **O resíduo não tinha para onde descer.**
Iterei seis vezes e ele mexeu 2,4% *por ruído*, e eu quase li isso como "quase convergiu".

**O sinal:** antes de otimizar contra um critério, perguntar **que entrada o violaria**. Se nenhuma
puder violá-lo, ele não é um alvo — é uma tautologia com um número ao lado. É a irmã da asserção que
não pode falhar, agora no papel de **função objetivo**.

O critério certo era a decomposição par/ímpar, que **não** é automática — e com ele o laço passou a
dizer alguma coisa (fechou em período 2).

Ver [[feedback-dois-caminhos]] — a mesma família: uma asserção mede um caminho contra um valor que
eu escrevi, e se eu errei a pensar ela confirma o meu erro.

## E o caso de 03/08, que é o mesmo defeito com álgebra por trás

Escrevi o controlo negativo da cifra assim: *com uma matriz de $\det\ne\pm1$, verificar que
`adj(B)·(B·v)` não é divisível por `det`.* **Não podia falhar** — `adj(B)·B = det·I`, portanto o
produto é `det·v` e é sempre divisível. A asserção media uma identidade da álgebra linear, não a
cifra.

O defeito real de `det ≠ ±1` é outro e é o interessante: **a cifra deixa de ser SOBREJETIVA**. Há
criptogramas sem origem inteira. Refeito, mede **50,0% contra 100,0%** — e `1/|det|` é exatamente o
fator de perda.

**O sinal novo, e é barato:** se a asserção tem a forma *"aplicar a operação e depois a inversa
dela"*, ela está a medir a definição de inversa. Perguntar antes: **o que é que este objeto perde
quando a hipótese cai?** Foi essa pergunta que trocou "não reverte" (falso) por "não é sobre"
(verdadeiro, e com número).

## 03/08, tarde — SETE asserções vazias num dia, e o teste que as decide

Três padrões novos, todos apanhados no mesmo dia:

| padrão | exemplo real |
|---|---|
| **identidade algébrica** | `adj(M)·M/det == I` — verdade para toda 2×2 |
| **divisão por ±1** | `x % det == 0` com `det=±1` — verdade para todo `x`. **Foi a minha CORREÇÃO da anterior** |
| **filtro a montante** | `if(a==b) continue` e depois "o espelho sai da fibra, 0 de 508" |
| **simetria da caixa** | varrer `[−60,60]²` e "descobrir" 3600 por quadrante |

**A regra que faltava:** uma asserção só mede se houver **uma população em que a resposta varie**.
Se todas as entradas dão o mesmo, não há medição. Ao refazer o `§P4` pela terceira vez, a versão
boa varre 2112 matrizes das quais **232** têm `det=±1` — a população discorda, e por isso mede.

## E o teste que decide: `tools/mutacao.sh`

Estragar o código e ver se a bateria acusa. Apanhou o que quatro revisores e a minha leitura não
viram: eu tinha **removido** a asserção vazia do `§P9` e, com ela, **a única cobertura do código da
decifra**. Mutei `K11*cp` → `K11*cp + 1` e tudo ficou verde.

> **Uma asserção pode ser vazia como AFIRMAÇÃO e ser o único teste de regressão de um bloco.**
> "Não prova teorema nenhum" ≠ "não serve para nada". Apagar a primeira abre um buraco invisível.

Cuidado com **mutações equivalentes**: `d = p/q` → `p/q+1` sobreviveu porque o
`if(r<0){ d--; r+=q; }` da linha seguinte a neutraliza. Distinguir antes de escrever asserção nova.

## E a regra mecânica, que é barata e eu não fazia

**Depois de corrigir uma frase falsa, `grep` pela frase EXATA nos três documentos.** Não pela
ideia — pela frase. Em 03/08 a mesma frase falsa vivia em dois sítios e eu corrigi um; a segunda
estava no *mesmo ficheiro*, cem linhas abaixo. Uma frase que vale a pena escrever costuma ter sido
escrita mais de uma vez.

**E a variante da CONSTRUÇÃO:** `tr(companion) == −c_{n−1}` parecia uma medição
e era tautologia: na companion **só a última entrada da diagonal é não nula**, e vale
`−c_{n−1}` por construção. O teste comparava o coeficiente consigo próprio. Estava assim
desde que o ficheiro nasceu, e nenhuma ronda o apanhou.

**O que o apanhou foi a SABOTAGEM** — e à segunda tentativa, porque a primeira também estava
errada: sabotar o COEFICIENTE não parte nada, porque dá só **outro polinómio**, onde o
teorema continua verdadeiro. O que parte é sabotar a **CONSTRUÇÃO** (a companion), que é o
que a tese afirma. Regra: *o gume tem de atacar o que a tese AFIRMA, e não a instância em
que ela é testada.*

**O conserto** é o de sempre — dois caminhos que não se tocam: `tr(A^k)` multiplicando
MATRIZES contra `P_k` pela recorrência de NEWTON sobre os COEFICIENTES. Com a companion
partida, os dois separam-se e caem duas asserções.

## 16/08/2026 — A NONA forma: a DEFINIÇÃO a fazer de medida

Numa só volta pela bateria, três ficheiros com a mesma forma — e nenhum tinha limiar
frouxo nem tipo errado. O defeito era a asserção **confirmar a própria escrita**:

```c
// arraytermico.c §A6 — a parcela definida como o RESTO
volta = P*e;  radia = P - volta;  soma = volta + radia;   ok(soma == P)   // P-volta+volta

// analog.c §B.2 — o produto reescrito
long malha = u1 * u2;   if (malha == u1*u2) passou++;                     // x == x

// aurea.c §A1 — a cópia comparada com a cópia
L cf[3] = {1,-1,-1};  L bd[3] = {1,-1,-1};  ok(cf[k] == bd[k])            // duas escritas
```

**O teste é sempre o mesmo: mudar o dado a montante e ver se o número se move.** Se `radia`
é o resto, mexer na eficiência não move nada. Se `bd` é escrito à mão, mudar `m` não o
move. Se a malha é o produto, não há o que mover.

E o conserto nunca é apertar a comparação — é **fazer o segundo lado derivar**:
`radia` de uma via própria (ou dizer que é identidade, e medir que a partição é uma
partição); `malha` do ANTILOG da soma dos expoentes; `bd` do `m` da borda.

**E o conserto costuma trazer a tese que faltava.** No `aurea.c`, com `bd` a derivar de
`m`, apareceu o gume que a versão anterior não podia ter: com `m = 2..5` a coincidência
CAI — logo `f' = f⁻¹` **escolhe** o ouro, e não vale para a família metálica toda. Isso
nunca estivera medido, porque os coeficientes não dependiam de nada.

O detector é o `tools/tectos.py` P3 — e a segunda forma (`v` comparado com a expressão que
o definiu) só entrou depois de o `malha == u1*u2` lhe escapar. Ver
[[feedback-a-referencia-escrita-a-mao]] e [[feedback-normalizar-nao-e-medir]].

## 16/08/2026 — o sítio onde eu baixo a guarda: o CONTROLO, e não a tese

Num dia inteiro a corrigir asserções vazias, escrevi CINCO. E não foram distribuídas ao
acaso — foram todas no mesmo sítio:

| o que eu estava a escrever | o que saiu |
|---|---|
| o gume de «Q_max depende do lado» | `if(w*h != h*w)` — a comutatividade consigo própria |
| o controlo da taxa do bairro | «o q medido não excede o DOBRO de 1/σ²» — passava por 0,142 ser pequeno |
| o controlo da identidade de Carnot | `if(X == X)` — a expressão comparada consigo própria |
| a contagem de um gume | 38416 onde eram 15⁴ = 50625 |
| a contagem de outro | 59535 onde eram 11·9⁴ = 72171 |

**Why:** a tese leva atenção porque é o que se quer provar. O CONTROLO — o gume, a segunda
rota, a contagem que impede a varredura vazia — é escrito depois, a correr, como
formalidade. E é exactamente ali que a asserção vazia nasce, porque um controlo que não
pode falhar tem a mesma cor de um que pode.

**How to apply:**
1. **Depois de escrever um gume, aplicar-lhe o gume**: que entrada faria ESTE controlo
   falhar? Se não houver, ele não é um controlo — é decoração.
2. **Nunca escrever um total à mão.** `11L*9*9*9*9` e não `72171`: a conta que não erra é
   a do compilador. Errei-a duas vezes num dia, e as duas eram gumes.
3. E o sinal operacional: se o controlo passou **à primeira** e sem me obrigar a pensar,
   é candidato a vazio. Os controlos verdadeiros costumam falhar uma vez antes de passar
   — foi o que aconteceu com «mais de metade não-nulas» no hopfield (errado por 4×) e com
   a perpendicularidade no cruzado_potencia (que NÃO é invariante).

Ver [[feedback-o-controlo-a-tres-linhas]], [[feedback-varrer-onde-nada-pode-falhar]].

## E a NONA forma: o valor EMPRESTADO que se finge medir

16/08, no `corpo_peano.c §CP13`. Havia `int n_leis = 8;` comparado com `8`, e eu fui
corrigi-lo contando: `for(k = 0; k < 16; k++) if(k % 8 == k) n_leis++;` — que conta
quantos inteiros de [0,16) são menores que oito, e dá oito **por aritmética modular**.

**Trocar uma tautologia por outra não é corrigir.** O ficheiro não tinha a lista das leis:
o oito vem da TEORIA, e nenhuma linha daquele bloco o pode contar.

**How to apply:** quando uma parcela da frase não se pode medir com o que o bloco tem:

1. **Tira-se da asserção.** Uma condição que não pode falhar não a fortalece — dá-lhe cor
   de medição.
2. **Diz-se de onde vem** — «da teoria», «do medidor X», «da literatura». Um valor
   emprestado é legítimo; um valor emprestado *disfarçado de medido* não é.
3. E se ele for usado pelo código, entra como o que é: no §CP13 o oito ficou como o
   **módulo do ciclo**, que é o que as contas de facto usam.

O mesmo vale para o `P_seebeck = 0,9931` do `colheita.c`, copiado do `cosmico.c`: um valor
copiado é POSTULADO, e a asserção que dizia «nenhuma é postulada» era falsa para ele.

## E onde isto me acontece: A CONSTRUIR O CONSERTO

Sete vezes em 16/08, e o padrão é o mesmo da secção anterior mas um passo à frente: não é
só no CONTROLO — é no acto de **corrigir**. A atenção vai toda para o defeito que se está
a tirar, e a peça nova entra sem passar pelo mesmo crivo.

**O sinal:** acabei de escrever um conserto e ele passou à primeira. Aplicar-lhe o gume
antes de commitar — foi assim que o `tikz.c` me apanhou (Cayley–Hamilton vale para TODA
matriz 2×2, logo verificá-lo com o traço dela não testava a companheira).
