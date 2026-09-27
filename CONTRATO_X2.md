CONTRATO X2 — ESPAÇO DE 2 BYTES
═══════════════════════════════════════════════════════════════════════════════

    Documento:  CONTRATO_X2.md
    Versão:    2026-09
    Âmbito:    definir X₂. Não define a seta.
    Estado:    FECHADO como contrato. A frente da seta NÃO está aberta por aqui.


0. NATUREZA E ESCOPO

Este documento define um espaço e as suas coordenadas. Nada mais.

Define:

  • o que é um elemento de X₂;
  • o que cada componente de um elemento pode valer;
  • as projecções π₁ e π₂;
  • o zero do segundo factor;
  • o oráculo O(b) = (b, 0);
  • o inventário do que existe, do que é representação, e do que não existe.

NÃO define, NÃO sugere, NÃO promete:

  • qualquer mapa de X₁ para X₂;
  • qualquer implementação em C;
  • qualquer adaptador;
  • a aritmética de X₂ que o paper não define.

As secções 1 a 5 são as definições. Elas não invocam nenhuma seta e não
dependem de nenhuma. A secção 6 é o teste dessa afirmação; a secção 8 é a
única que compara com o paper. Ambas estão fora das definições, e ambas
têm de estar: uma porque nomeia o que procura, a outra porque é material
externo.

A distinção é verificável sem qualquer confiança no autor: reler 1 a 5 e
confirmar que nenhuma delas recorre a 6, a 8, a 9 ou a qualquer código. O
resultado da verificação está em 6.


1. DEFINIÇÃO

  X₂ := (Z/256Z)²

Um elemento de X₂ é um par ordenado de classes de resto módulo 256:

  x = ([b₀], [b₁])

onde [b] denota a classe de resto de b em Z/256Z, e b₀, b₁ são quaisquer
inteiros. Escrever x = (b₀, b₁) sem parênteses de classe é o mesmo objecto
escrito com representantes canónicos; ver a obrigação de bem-definido em 2.2.

A ordem dos componentes é a ordem de escrita: primeira posição, segunda
posição. O que ocupa a primeira posição é o que 3 chama π₁.

Cardinalidade:

  |X₂| = 256 · 256 = 65536 = 2¹⁶

Nenhuma estrutura adicional é assumida. X₂ tem, por enquanto, coordenadas e
nada mais: nem corpo, nem grupo, nem anel. As operações possíveis estão
inventariadas em 7 e a maior parte não existe.


2. REPRESENTAÇÃO SEMÂNTICA

2.1 Os dois componentes

  • Primeiro componente: um elemento de Z/256Z, isto é, uma classe de resto
    módulo 256. Assume um de 256 valores, canonicamente 0, 1, …, 255.

  • Segundo componente: também um elemento de Z/256Z, também com 256 valores
    possíveis, também canonicamente 0, 1, …, 255.

  • Os dois componentes são independentes. Não há acoplamento, nem relação,
    nem restrição entre b₀ e b₁ que reduza as 65536 combinações.

  • O par é ordenado. (b₀, b₁) e (b₁, b₀) são o mesmo elemento se e só se
    b₀ = b₁.

2.2 Obrigação de bem-definido

Uma vez escrito com classes, x = ([b₀], [b₁]) não depende dos inteiros
escolhidos: ([b₀], [b₁]) = ([b₀ + 256], [b₁ − 256]) e afins.

Sob a convenção de representantes canónicos 0…255 esta obrigação é
automática, porque o representante é único. Ela deixa de ser automática se
alguma vez for adoptada uma representação não canónica, e nesse caso passa a
ser uma asserção a medir. Registado em 10.

2.3 Representação computacional

Este contrato NÃO escolhe representação de memória. Escolhe representação
semântica: par ordenado, dois componentes independentes, 256 valores cada.

A representação de memória é uma decisão de implementação, e as condições
que ela tem de satisfazer estão em 7.3. Ela fica aberta em 10.


3. PROJEÇÕES

Definidas sobre X₂, independentemente uma da outra e de qualquer seta.

  π₁ : X₂ → X₁,    π₁( ([b₀], [b₁]) ) = [b₀]

  π₂ : X₂ → X₁,    π₂( ([b₀], [b₁]) ) = [b₁]

Cada uma lê uma posição e devolve um elemento de X₁, que é uma cópia de
Z/256Z. As duas projeções são funções totais: definidas em todos os 65536
elementos. Nenhuma exige conhecimento de nada para além das coordenadas.

Os valores de X₂ são exactamente os pares de bytes, 256 por 256, e a
bijeção está em 7.2. É por essa bijecção que as duas projeções, juntas,
separam os 65536 elementos uns dos outros.


4. ZERO DO SEGUNDO FATOR

O segundo factor de X₂ é uma cópia de Z/256Z. O seu elemento nulo é a classe
de resto de 0:

  0₂ := [0] ∈ Z/256Z,   occupying a segunda posição do par.

Nota que o valor 0₂ e o zero da primeira posição são o mesmo número: 0. O que
os distingue é a POSIÇÃO, não o valor. Não há, portanto, um objecto diferente
a fabricar para o segundo factor — o mesmo resíduo 0 é reutilizado, colocado
na segunda casa.

O elemento nulo de X₂, por sua vez, é o par (0, 0), com 0 nas duas posições.


5. ORÁCULO INDEPENDENTE

Para qualquer b ∈ X₁, o resultado esperado é

  O(b) = ( [b], 0₂ )

isto é: o par cuja primeira posição é a classe b, e cuja segunda posição é o
zero do segundo factor.

O oráculo é construído a partir de:

  • a definição de X₂ como par de duas posições  (§1);
  • o valor de b, que já está em mão  (§5, primeiro componente);
  • o zero do segundo factor, definido em §4 sem seta nenhuma.

Não há, na construção de O(b), qualquer passo que requeira conhecer um mapa
de X₁ para X₂. Não se chama função alguma. Não se consulta implementação.
Não se deriva de nada que ainda não esteja escrito acima.

A construção de O(b) é válida para os 256 valores de b, sem excepção.

Limitação honesta: O(b) e o eventual mapa de X₁ para X₂ coincidem por
definição do alvo, não por raciocínio independente. O oráculo é uma
re-derivação a partir da definição do espaço, não um cálculo por algoritmo
diferente. O que ele pode provar é se a implementação concorda com a
definição; não pode provar que a definição é a única possível. Isto está
alinhado com a nota já registada sobre a independência do oráculo em §4a da
cadeia composta, que foi partial e não total.


6. TESTE DE ACEITAÇÃO

O contrato é aceito se, e só se, as três definições abaixo podem ser escritas
sem mencionar a seta uma única vez. São elas, na íntegra:

    π₁(x) = primeira posição de x
    π₂(x) = segunda posição de x
    O(b)  = par com b na primeira posição e o zero na segunda

Nenhuma das três recorre a §8, a §9, a qualquer mapa, ou a qualquer
código. Este é o critério de aceitação.

Não fica por confiar na leitura. Foi verificado mecanicamente:

  • detector: procura o nome em ASCII e o ponto de código U+03B9;
  • detector auto-testado ANTES de usado, contra uma linha de prova. Casou
    com as duas formas. Um detector que não casa não pode reportar zero, e
    um que não foi testado pode reportar zero sem nada ter contado;
  • ocorrências da seta em §1 a §5, que são as definições:  ZERO;
  • fora das definições, a seta aparece em §7, §8 e §9, em material de
    inventário, de conferência e de viabilidade de controlo.

A leitura é §1 a §5 e não §1 a §6 porque esta secção tem de NOMEAR o
que procura para reportar sobre ele. São três as ocorrências que o detector
encontra dentro desta secção, e as três são de relatório: a linha do padrão
de busca, a do iota grego dito por extenso, e a da contagem inflatada de C.
Contá-las como se fossem uso da seta seria falso; escondê-las seria pior.
São três, e o critério é sobre as definições.

Recontar: duas linhas de qualquer lado, e repetir o auto-teste antes de
acreditar no zero.

Duas vezes nesta frente o "zero" esteve errado antes de estar certo, e as
duas vezes por causa do instrumento e não do objecto. A primeira: o iota
grego não passou pelo console, e o detector/reportou zero sem nunca ter
casado com nada. A segunda: uma contagem inicial de "X_d", "C_d" e
"iota" em C, feita sem fronteira de palavra e sem caixa, devolveu 78, 55 e
21; a repetição correcta devolveu 1, 0, 0, e o 1 era um comentário meu.
Um zero que não vem com o auto-teste ao lado é um número sem canal de
evidência.

Consequência para o medidor, quando vier: o valor esperado de um caso é
construído a partir da definição do par, e não obtido chamando a função
auditada.


7. OPERAÇÕES — INVENTÁRIO

Três categorias, separadas. Confundir qualquer duas delas é o erro que
o contrato existe para impedir.

7.1 Já definido matematicamente, no paper

  • X_d = (Z/256Z)^d, e |X_d| = 256^d                    campos.tex:382-395
  • C_d, codificação inteira posicional, bijecção para
    {0, …, 256^d − 1}:  C_d(b₀,…,b_{d−1}) = Σ b_k 256^k   campos.tex:390-393
  • S_d, sucessor com carry                              campos.tex:399-411
  • C_d(S_d(x)) = C_d(x)+1, com x = (255,…,255) a dar (0,…,0)
                                                        campos.tex:413-417
  • carry interno, e overflow global                      campos.tex:467-475
  • três distâncias: d_C, d_H, d_hier                    campos.tex:506-511
  • a órbita de S_d é a travessia lexicográfica induzida por C_d
                                                        campos.tex:496-502

  Para d = 2 em particular, o paper já dá:

    C_2(b₀, b₁) = b₀ + 256·b₁,  bijecção para {0, …, 65535}

7.2 Representação computacional já sancionada pelo paper

  • X₂ tem exactamente 65536 elementos, e C_2 é uma bijecção para
    {0, …, 65535} = [0, 65535]. Um inteiro de 16 bits sem sinal contiene
    exactamente o mesmo número de valores.

  • Portanto a codificação de X₂ por um inteiro de 16 bits é a própria C_2 do
    paper, não uma invenção deste contrato. Não é uma escolha nova; é uma
    leitura da escolha já feita.

  •   Nada se perde ao representar por código de 16 bits. De
    C_2(b₀,b₁) = b₀ + 256·b₁ e da injectividade de C_2 segue exactamente

        b₀ = C_2(x) mod 256        b₁ = C_2(x) div 256

    e portanto π₁ e π₂ são recuperáveis do código sem perda. A escolha
    entre "par de bytes" e "código de 16 bits" affects a forma, não o poder
    de medida. Fica em 10 como decisão, não como risco.

7.3 O que NÃO existe

  Não definido no paper, e por isso não affirmed aqui:

  • addition, subtracção, multiplicação em X_d. O paper diz que a aritmética
    posicional em base 256 é estrutura clássica, e formaliza a
    REPRESENTAÇÃO, o sucessor e a extensão dimensional. A aritmética de grupo
    ou de anel sobre X_d não está formalizada.
  • π₁ e π₂. O paper não define projeções de coordenadas. Os π que lá
    aparecem são de outra coisa: a realização π_S : I → S, e a sequência
    π_d(n) = S_d^n(x₀). Este contrato é que preenche essa lacuna, e preenche-a
    sem seta.

  Não implementado em C, verificado por busca com fronteira de palavra e
  case-sensitive sobre lib/, tests/, banco/:

  • X_d ....... 0 implementações
  • C_d ....... 0 implementações
  • S_d ....... 0 implementações
  • π_k ....... 0 implementações
  • qualquer mapa de X₁ para X₂ ... 0 implementações

  A única ocorrência de "X_d" em código C é um comentário em
  tests/cadeia_word8_mat2q.c:98, que remete para o paper. Não é
  implementação.

  Nota de método, porque quase fooled: uma busca inicial por "X_d",
  "C_d" e "iota" com comparação sem caixa e sem fronteira de palavra devolveu
  78, 55 e 21 ocorrências. Todas eram ruído: "x_d" casa dentro de
  identificadores como p2_int_dx_dy, e "c_d" dentro de nomes menores. A
  repetição com fronteira de palavra e case-sensitive devolveu 1, 0, 0. O
  único número que sobreviveu foi o do comentário. Uma contagem que não
  sobrevive a refazer a pergunta não entra num contrato.


8. ALINHAMENTO COM O PAPER — não usado nas definições

Tudo o que está nesta secção pode ser contradito por uma mudança no paper sem
que as secções 1 a 6 deixem de estar correctas. Está aqui para conferência,
não para derivação.

  • O paper define, em campos.tex:479-481, uma inclusão entre espaços de
    dimensões diferentes, que appende um zero no extremo de índice mais alto:

        (b₀, …, b_{d−1}) ↦ (b₀, …, b_{d−1}, 0)

    Com a numeração do paper, b₀ tem o peso 256⁰ e é o menos significativo, e
    o zero acrescentado ocupa o índice d, o mais significativo. Logo, para
    d = 1, o par resultante tem b na primeira posição e 0 na segunda — a
    mesma forma que o oráculo de §5.

  • Isto é conferência de coerência, não derivação. O oráculo de §5 não
    precisa desta secção: ele sai de §1, §4 e do valor de b.

  • O paper, em campos.tex:1369, já regista que a extensão dimensional não é
    a Transformada. E em campos.tex:557 já recusa a identificação
    "2⁸ = 256" como resultado, chamando-lhe reformulação de cardinalidade. A
    colisão nominal de símbolos entre o X dos papers e o X/F₈ do código
    permanece PENDENTE e não é tocada por este contrato.

  •   Este documento é sobre ESPAÇO. A inclusão do paper é sobre DIMENSÕES.
  São coisas diferentes, e o paper também o diz.


9. CONTROLES NEGATIVOS — VIABILIDADE

Pergunta: é possível construir, sem depender de nada que este contrato ainda
não definiu, uma perturbação que viole cada um dos dois invariantes
separadamente?

Resposta: sim para os dois, com uma restrição de domínio num deles.

  Canal 2 — violar π₂∘ι₁ = 0 mantendo π₁∘ι₁ = id:

      (b, b)

    Primeira posição correcta, segunda posição errada. Viola exactamente um
    invariante. Construível a partir de §1 e §3, sem seta, sem código novo.
    Domínio genuíno: b ∈ {1,…,255}. Para b=0, (0,0) coincide exactamente
    com o oráculo O(0)=(0,0) — não é perturbação, é o próprio oráculo.
    Não contar b=0 como controle.

  Canal 1 — violar π₁∘ι₁ = id mantendo π₂∘ι₁ = 0:

      (S₁(b), 0)

    Segunda posição correcta, primeira posição errada. Viola exactamente um
    invariante. Construível sem seta, desde que "b mais um" seja lido como o
    sucessor S₁ do paper, que está definido em campos.tex:399-411 e que para
    d = 1 dá S₁(b) = (b + 1) mod 256. O nome "b+1" não é operação que o paper
    defina; S₁ é.
    Domínio genuíno: b ∈ {0,…,254}. S₁(255)=0 invalida o controle (coincide
    com o oráculo de b=0); fica fora por periodicidade.

  Mesma classe de fenômeno das duas correcções:

    S₁(255)=0  pode coincidir com o oráculo de outro input;
    (0,0)=(O(0)) coincide com o próprio oráculo de b=0;
    em ambos os casos há coincidência entre casos de entrada,
    não ponto fixo da aplicação nem falha do mapa.

  A restrição: S₁(255) = 0, pelo overflow global. Logo, em b = 255, a
  perturbação (S₁(255), 0) = (0, 0) deixa de ser uma perturbação: coincide com
  o par que é a resposta correcta para b = 0. O canal 1 só tem força para

      b ∈ X₁ \ {255}

  Fora de 255, S₁(b) ≠ b, e a perturbação é genuína. Este é o mesmo cuidado
  que o registado para a terceira medição instrumentada, e é uma coincidência
  estrutural, não uma coincidência de números: S₁(b)=b implicaria 1≡0 (mod 256),
  o que é impossível — o sucessor numérico não tem ponto fixo. O que tem é a
  perturbação: em b=255, S₁(255)=0, e (S₁(255),0)=(0,0) coincide com o oráculo
  de b=0, não consigo distinguir perturbação de resposta correcta nesse ponto.
  É a periodicidade do sucessor, não ponto fixo da aplicação.

  A construção de S₁ em C, quando vier, não precisa de adaptador novo e não
  pode chamar a seta: o sucessor sobre naturais já existe em lib/naturais.h:44
  (nt_S), e a redução módulo 256 já existe em lib/naturais.h:148
  (w8_proj_wrap, que devolve o resíduo por narrowing). Composição das duas dá
  S₁ sem tocar na seta. Há, porém, um efeito lateral a gerir — w8_proj_wrap
  conta em w8_wrap. Registado em 10, não resolvido aqui.


10. DECISÕES ABERTAS

Nenhuma destas é bloqueante para o contrato. Todas são para a frente seguinte.

  1. Representação de memória de X₂ no medidor: par de dois bytes, ou código
     de 16 bits (que é a C_2 do paper). 7.2 mostra que a escolha não altera o
     poder de medida; altera o que o medidor toca com as mãos.

  2. Domínio de b no canal 1. Excluir 255, ou cobri-lo com um caso separado
     que assinale a degeneração em vez de a esconder.

  3. Efeito lateral de w8_proj_wrap sobre o contador w8_wrap. Usar a redução
     existente e explicar o delta, ou construir a redução à parte. Decisão do
     medidor, e ela tem de ser tomada antes de correr, não depois.

  4. Medir ou não a independência de representante (2.2). Só é obrigatório se
     alguma representação não canónica entrar. Hoje, com representatives
     canónicos, é automático e não há o que medir.

  5. Colisão nominal X/F₈. PENDENTE, fora do âmbito, não tocada.


11. ONDE CADA COISA FECHA

    o que é X₂            §1, par de duas classes de resto
    o que cada posição vale §2.1, 256 valores, independentes
    bem-definido           §2.2, canónico faz automático
    π₁ e π₂               §3, primeira e segunda posição
    o zero do segundo      §4, o mesmo resíduo 0, noutra posição
    o oráculo             §5, (b, 0), sem seta e sem código
    aceitação             §6, as três definições sem mencionar a seta
    o que já existe        §7.1, paper
    o que é representação §7.2, C_2 do paper
    o que não existe       §7.3, nem a aritmética nem qualquer código
    controlo do canal 2    §9, (b, b), para todo o b
    controlo do canal 1    §9, (S₁(b), 0), para b ≠ 255

    A seta não aparece em nenhuma linha de §1 a §6. É para isso que este
    documento existe: o alvo tem de ser observável antes de haver algo que
    atravesse-o.

═══════════════════════════════════════════════════════════════════════════════
