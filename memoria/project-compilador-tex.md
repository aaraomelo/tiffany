---
name: project-compilador-tex
description: "A tradução de formato (tex.c, spline.c, chessb.c): .tex→PDF, a largura pela curva, e o WASM/Node ingeridos — quatro roupas novas e ZERO lugar novo"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T17:35:49.902Z
---

# A tradução de formato — `tools/tex.c`, `tools/spline.c`, `tools/chessb.c`

**01/08/2026 (noite), onze commits `5613e65`→`c7b11c7`** — 207 medidores, 205 verdes, 0 falhas,
tudo no ar e o fork sincronizado. O CI do chess falhava por faltarem pacotes LaTeX
no runner. Eu ia resolver com o container `texlive/texlive` — **uma linha**. O Aarão insistiu no
compilador próprio, e a razão dele muda a natureza da coisa:

> *"a assistente vai precisar compilar os .tex, senão como teremos os notebooks?"*

Não é contornar o CI. É uma peça que a assistente precisa de ter. O deploy do chess ficou pausado
durante o trabalho e **subiu verde no fim** — ver [[project-publicacao-patria]] e
[[feedback-o-disco-limpo]].

## Nada abriu lugar novo — e essa é a prova de que estava certo

| peça | de onde veio | o que faz |
|---|---|---|
| a **descida** | `caminho.h` | o LaTeX marca o nível com a barra, como JSON com o parêntese |
| o **léxico** | `traduz.c` §T1 | comando → glifo, 100 pares, e a **volta é a mesma tabela ao contrário** |
| o **prismático** | `prisma.c` | *"o triângulo deformado até preencher a área"* **é** justificar a linha |
| **solar/lunar** | `koch.c` | o solar guarda a estrutura, o lunar desenrola a página |
| o **shell** | `sql.c` | `ABRE`/`STORE`/`LOAD`/`MEDE` — o PDF é backend, como martelo/canal/pool |

`tex.c` **22 unidades**, `spline.c` 15, resíduo 0. Zero dependências. PDF sem compressão, para o §X6 o
ler de volta. **Bateria: 203 medidores, 201 verdes, 0 falhas.**

## OS DOIS BUGS, que são o mesmo bug por duas portas

**(1) O cifrão no verbatim.** Linha 1532 do `catalogo.tex`: **`$ MARTELO 2083236890`**. Aquele
cifrão é um *prompt*; eu li-o como delimitador. Número **ímpar** → o modo matemático ficava ligado
**até ao fim do documento**.

**(2) O acento.** Com o modo ligado eu mandava para a Symbol tudo o que `isalpha()` aceitasse. O
`ç` é `0xE7` e `isalpha(0xE7)` é falso em C locale — então **`coração` saía `cora` na Symbol e
`ção` na regular**. Dois `Tj`. Nos bytes via-se: `b'cora) Tj ET'`.

**Nos dois: o texto estava lá e a PALAVRA é que tinha deixado de existir.** Custou 159 de 2240
palavras do catálogo.

As curas: o verbatim é literal; só ASCII vai para a Symbol; e **o modo matemático não atravessa
parágrafo**, como no próprio TeX (*"Missing $ inserted"*) — assim um cifrão solto danifica um
parágrafo, não o resto do documento.

    catálogo: 159 de 2240 -> 5 de 2240 (99,78%)    teoria: 7 de 1197 (99,42%)
    e o que sobra são nomes de ambiente (itemize, tabular) e um label

**A lição que fica: um estado que só LIGA e nunca desliga sozinho não falha — apaga. E o dano não
nasce onde aparece:** nascia na linha 1532 e eu procurava-o na 2071. Achei-o por **bissecção sobre
o documento** — cortar o fonte ao meio e ver de que lado o defeito vive. É a técnica a repetir.

## As splines — e o que o oráculo externo provou

O contorno TrueType **já é** spline: `B(t) = (1−t)²P₀ + 2t(1−t)P₁ + t²P₂`, e no projeto todo dado
já é polinômio na base. Não houve conversão, houve leitura; a TTF é lida à mão, sem biblioteca.

**A Liberation Sans é metricamente compatível com a Helvetica**, logo o ficheiro arbitra a tabela
base-14 que eu tinha copiado: **95 de 95 batem, nas duas variantes.** O `@` é 1015 regular e 975
negra, tal como a tabela dizia.

**A tabela estava CERTA.** O que estava errado era a afirmação que eu fiz sobre ela. *O erro nunca
foi o número: foi inventar uma lei em vez de medir a que havia.* Ver [[feedback-assercoes-vazias]].

A área sai por Green, e sobre uma quadrática o integrando é grau 2 em `t` — integra-se **exato**.
Derivei em vez de decorar, porque **uma fórmula decorada é a tabela copiada outra vez**.

## A tríade nos espaçamentos, e ela caiu sem se forçar

    o PASSO  é aditivo        -> ⊕ FOURIER   uniforme é o modo zero (2e-23 fora dele);
                                             dobrar o corpo dobra o modo 0 e QUADRUPLICA
                                             a energia, que é quadrática
    a ESCALA é multiplicativa -> ⊗ MELLIN    escala vira potência; em log é translação
                                             pura, log(1,2)=0,182322 medido na página

Sem vazamento: pior resíduo **2 milésimos de ponto em 451000**.

## A largura passou a vir da CURVA (`spline.h` ligado ao `tex.c`)

A leitura da TTF saiu para **`tools/spline.h`** — uma leitura, dois usos. *Duas cópias da mesma
leitura são dois sítios onde a correção pode chegar só a um.* A `largura()` do `tex.c` consulta a
curva quando a fonte está no sistema, cai na tabela quando não está, **e diz qual usou**.

**O ganho não é o ASCII, que já batia — é o português:**

    ASCII        55 exatos, 40 a 1 milésimo (a divisão por upem=2048), 0 piores
    acentuados   6 larguras distintas, de 277 a 777 — a tabela dava 556 a TODOS
    na página    uma linha de português muda 130 milésimos (0,43%)

E 0,43% num parágrafo inteiro é exatamente **onde a linha quebra**.

## O chess ingerido: WASM e NODE, e a ISA não cresceu (`chessb.c`)

> *"agora ingere o chess no banco, ora isso precisa descer na ISA de novo e ver backend
> webassembly e node — a assistente vai precisar criar apps."*

O chess já tinha os dois no disco (cinco `.wasm` e o app em Vite). **Nada a inventar.**

**O WASM é a roupa mais limpa que entrou**, e por uma razão que importa: o formato é
**auto-descritivo por construção** — cada secção diz o seu tamanho *antes* do corpo, então descer é
somar. **É o que o banco faz com os slots**, e é por isso que ele desce na ISA sem tradutor.

    5 módulos, 31 secções, todas com 'code'; A DESCIDA FECHA (a soma dá o ficheiro, sem sobra)
    decidir.wasm (172 B): type(5) function(2) memory(3) export(17) code(90) custom(35)

O **Node** não é sequer roupa nova: `package.json` é JSON, e o JSON está no `caminho.h` desde
sempre. **Zero código novo — e isso É o resultado.**

**E A ISA JÁ ERA DE PILHA.** Ela faz `LOAD` como `B←A ; A←mem[slot]` — deslocar A para B e pôr o
novo em A **é literalmente um push numa pilha de dois**. Simulando a pilha sobre os onze pares da
tradução, ela **nunca precisa de mais de dois**: os seis binários do wasm caem todos na ULA sobre
`(A,B)`, e o `R` é onde o `STORE` grava. *A pilha estava lá, escrita por extenso.*

A **ingestão é pela cifra**, não por cópia: 9 peças, 9 cifras distintas, e a cifra **reconstrói o
tamanho exato** — coordenada, não etiqueta.

**Criar apps:** o mínimo medido são três ficheiros (manifesto, entrada, dados) e os três formatos já
estão no catálogo. **Não é capacidade nova, é receita.**

## A lista das roupas, e ela só cresce de um lado

    JSON      o nível é o parêntese              [ { } ]        caminho.h
    YAML      o nível é a indentação                            caminho.h
    Markdown  o nível é a contagem de            #              caminho.h
    LaTeX     o nível é a barra e o nome         \section       tex.c §X1
    PDF       o nível é o objeto e a xref                       tex.c §X5
    WASM      o nível é a SECÇÃO                 id+tam+corpo   chessb.c §C2
    Node      — não é roupa nova: é o JSON                      chessb.c §C3
    SSH       o nível é o COMPRIMENTO            uint32+pad     sshb.c §H1
    TikZ      DUAS marcas: ';' e a chave          e ela CALCULA  tikz.c §K1

**Nove formatos, uma descida.** Cada um novo é uma linha de tabela, não um analisador. E há duas
famílias: os que **dizem o tamanho à cabeça** (WASM, SSH — descer é somar) e os que **obrigam a
procurar o fecho** (JSON, LaTeX). O nosso banco é da primeira, e é por isso que ela desce sem
tradutor.

## O TikZ: a roupa que CALCULA, e a figura que pode desmentir o texto (`tikz.c`)

> *"traz o MATLAB, conecta com latex e faz animações e simulações, e ingere o Tikz pra trazer a
> dinâmica. Vê a linguagem de programação do latex e usa ela pra validações dos sistemas dinâmicos
> do corpo diferencial."*

**Nona roupa, e a primeira em que a roupa calcula.** O TikZ tem laço (`\foreach`), aritmética
(`\pgfmathsetmacro`), condicional e definição — as quatro peças de uma linguagem.

**E é isso que muda a natureza da figura.** O `.tex` gerado **não traz os pontos escritos: traz a
regra que os produz**, e integra a equação outra vez ao compilar.

> *Se os pontos estivessem escritos, a figura seria um retrato do que eu calculei e não teria como
> discordar de mim. Uma figura que não pode desmentir o texto não prova nada sobre ele.*

**Os dois caminhos, por métodos DIFERENTES de propósito:** o C integra `y''+By'+Cy=0` (`edo.c` §E1,
Δ<0, elíptico) por **RK4**; o TikZ por **Euler**, que é o que o `pgfmath` faz sem esforço. *Se ambos
fossem RK4 eu estaria a comparar duas cópias do mesmo código.* Compilado com `\typeout`, o pdflatex
cospe os **seus** valores no log — oráculo externo, como a Liberation Sans:

    passo    LaTeX        C        desvio
      20    0,92755    0,92741    1,4e-4
     100   -0,19720   -0,19762    4,2e-4     <- a precisão do pgfmath (ponto fixo)

**Um PDF de N páginas É a animação**, sem biblioteca nenhuma (8 páginas, `pdfinfo` confirma).

**O MATLAB não trouxe corpo novo — trouxe notação.** A transposta é o `J` (involução, medida), a
inversa é o `NEGRO_OURO`, e a régua da companion é a `(B,C)=(−traço,det)` do catálogo, conferida.

## Cinco domínios, uma equação (`dominios.c`)

> *"desenhar circuitos elétricos e mecânicos no TikZ, converter em equações, resolver passo a passo
> e animar. Inclui pneumático, óptico e o novo, o elástico via corpo mórfico. E resgata o backend em
> PTX — aí pode receber sinal de GPU e CPU, por uma janela nossa semelhante ao canvas."*

**Elétrico, mecânico, pneumático, óptico e elástico não são cinco sistemas parecidos: são o mesmo
corpo com cinco vestidos.** Todos caem em `y''+By'+Cy=0` (o `edo.c` §E1), e **o `(B,C)` não está
escrito — sai dos parâmetros físicos** `(m,c,k)`. Cinco réguas distintas, todas elípticas, e a
**mesma** forma fechada satisfaz os cinco por substituição: resíduo relativo `7e-9`. *Nenhum
precisou de caso especial.* Os cinco `.tex` compilam: 2 páginas cada, o circuito e a curva.

**O elástico pelo mórfico:** `abertura ≤ u ≤ fecho` ponto a ponto, e a **idempotência**. *Elástico é
a parte que volta; PLÁSTICO é a que a idempotência reteve* — e essa fica na garrafa até ter dual.

**A janela do PTX não é peça nova:** `ld.global`/`st.global` são `LOAD`/`STORE` sobre um slot. A
"janela semelhante ao canvas" é um **buffer de slots** que a GPU e a CPU escrevem, e o banco não
sabe quem escreveu — como o martelo, o canal e o pool.

## O SSH contra o bump: as voltas, e a medida que não é opinativa (`sshb.c`)

Nasceu de uma falha real: o `publica.yml` morreu com **`Network is unreachable`** — o runner
resolvia o host em IPv6 e não tinha rota, e falhou **antes de haver ligação**. Curado com
`AddressFamily inet` no `~/.ssh/config`. **É esse *antes-de-haver-ligação* que o medidor mede.**

**A medida:** numa sequência de mensagens, cada **inversão de direção** gasta meia volta de rede, e
nenhuma implementação pode entregar o primeiro byte antes de a luz ir e voltar. Contar inversões é
uma medida do **protocolo**, não de uma implementação — e os números de mensagem vêm dos
RFC 4253/4252/4254, oráculo externo.

    SSH    19 mensagens   17 inversões   8,5 voltas
    bump    1 mensagem     0 inversões     0 voltas

E não é uma constante: é **8,5× o rtt, sempre**. Boa Vista→São Paulo, 510 ms; satélite, 5,1 s.

**A conta é dos DOIS lados, senão era propaganda.** Cinco coisas só o SSH faz (falar com quem não
conhece, negociar algoritmo, autenticar servidor nunca visto, sobreviver a rechave, atravessar
NAT); duas só o bump (chegar sem voltas, não ter um *antes-da-ligação* onde falhar).

> **O SSH paga voltas para falar com um DESCONHECIDO. O bump não paga porque não fala com
> desconhecidos** — a banda **é** a assinatura do tecido (`canal.c` §N1), e quem não a tem nem
> decodifica.

*Um medidor que só conta a favor de um lado não mede: escolhe.*

## Um defeito dentro do próprio instrumento

O `MEDE` do shell comparava a mesma agulha com o `.tex` (UTF-8) e o PDF (WinAnsi): dava
`fonte 0 -> PDF 7`, impossível, e o resíduo saía 0 porque `0 > 7` é falso. **Uma medida que não
pode falhar não mede — e desta vez estava dentro do medidor.** Agora a agulha traduz-se para cada
lado, e o caso degenerado diz que não diz nada.

E os dois contavam **uma unidade grossa** cada por não emitirem o `#UNIT` que a `bateria.sh` lê —
a soma mostrava 2 onde havia 32.

## O balanço da série, e é ele que vale guardar

Começou num bug de CI que eu ia curar com **uma linha** (o container `texlive/texlive`). O Aarão
insistiu no compilador próprio pela razão certa — *"a assistente vai precisar compilar os .tex"* —
e daí saíram seis medidores e **nove roupas numa descida só**.

**O padrão que se repetiu em todas:** cada peça nova provou **não ser nova**. E três delas
devolveram coisas que já estavam no repo **sem nome**:

- a **ISA sempre foi de pilha** — `LOAD` faz `B←A ; A←mem`, que é um push de dois, escrito por
  extenso; ninguém lhe tinha chamado isso
- os **formatos sempre foram duas famílias** — os que dizem o tamanho à cabeça (WASM, SSH) e os que
  obrigam a procurar o fecho (JSON, LaTeX); a lista só o mostrou à oitava roupa
- os **cinco domínios sempre foram um corpo** — o Aarão pediu pneumático, óptico e elástico
  esperando cinco casos, e não há cinco casos: há uma régua `(B,C)` e cinco leituras

E o que parecia trazer teoria (MATLAB, PTX, o canvas) trouxe **notação sobre coisa que já existia**.

**O meu dia foi um erro só, em seis caras:** quatro números escritos de cabeça, um sinal invertido,
uma escala copiada de outro medidor. Os três tipos estão nos memories há semanas — e o que falhou
não foi escrevê-los, foi **consultá-los antes**. Ficaram com gatilho, não só com lição:
[[feedback-assercoes-vazias]] (*literal numérico que não calculei ali? parar*) e
[[feedback-simulacao-nao-bate]] (*parâmetro copiado de outro medidor? verificar a escala de
origem*).
