---
name: feedback-o-numero-no-veredicto
description: "Escrevo o texto do ok() com os números ANTES de correr — três vezes num dia, e a bateria passa verde na mesma porque o veredicto é uma string."
metadata:
  type: feedback
---

Escrevo o bloco, escrevo o `ok("… 15/15 … 30 coeficientes …", mal == 0)`, corro, e
os números reais são **10/10 e 20**. A bateria dá **verde**: a condição é
`mal == 0`, e o texto é uma *string* que nenhuma asserção lê.

Três vezes no mesmo dia: §W94 («15/15» → 10/10, «30» → 20), §W98 («100 de 108» →
34 de 192), §W103 («143/143» → 77/77, «3220 … 876» → 2401 … 1695).

**Why:** o `\medido` e o veredicto são o que o paper cita e o que fica no commit.
Um número errado ali é uma **afirmação falsa publicada**, e o gume não a alcança
por construção — o medidor mede o objecto, não o que eu disse sobre ele. É a
[[feedback-a-referencia-escrita-a-mao]] a reaparecer no sítio onde menos se vê.

**How to apply:**
1. **O veredicto escreve-se DEPOIS de correr.** Deixar o `ok()` com o texto
   qualitativo, correr, ler os contadores, e só então pôr os números.
2. **Nunca antecipar uma contagem** «óbvia»: `n=2..6 × m=1..3` parecem 15 e são 10
   quando o regime muda; 4096 triplos parecem repartir-se de uma maneira e
   repartem-se de outra.
3. **Ler a linha do medidor e a linha do veredicto lado a lado** antes de commitar
   — é uma comparação de dois segundos e apanha tudo.
4. E o mesmo vale para o `\medido` do `.tex`: ele copia os números do veredicto, e
   o erro propaga-se do medidor para o paper sem ninguém o ler.

Ver [[feedback-o-medido-sem-medidor]], [[feedback-assercoes-vazias]] e
[[feedback-o-grep-que-nao-conta]] — os três são a mesma família: o instrumento
certo a atestar a coisa errada.

---

## 27/09/2026 — a quarta vez, e o compilador que já tinha dito

Medidor novo (`tests/cadeia_word8_mat2q.c`, cadeia `Word₈ → E₁₆ → Qz → Mat →
Mat2Q`). Escrevi duas asserções com `%d` no texto, esperando que o `ok()` o
substituíse: `ok(…, "… nos %d casos medidos", dentro == compostos)`.

**`ok()` não aceita varargs.** A assinatura é `ok(const char *pergunta, int
cond)` — o texto é uma *string* e não há por onde injectar o número. A
execução imprimiu `%d` literal, e o veredicto deu **verde**.

Desta vez há uma variante que vale mais que o erro: **o `gcc` avisou.**
`-Wformat: format '%lld' expects argument of type 'long long int', but argument
6 has type 'char *'`. O aviso estava lá, exacto, com a linha e a coluna.

Só que:

1. vinha **depois de ~100 linhas** de `-Wunused-function` (cabeçalhos que
   declaram muito e são usados pouco);
2. e o PowerShell tinha acabado de embrulhar o stream **inteiro** em
   `NativeCommandError`, porque escreve o banner de compilação em **stderr**.

A minha primeira leitura foi, portanto, «isto falhou» — e não «isto é um aviso de
formato entre cem». Se eu tivesse lido só a primeira e a última linha e ignorado
o meio, `%d` ia para o disco e para o mapa, **verde**.

**Why (o que acrescenta a este ficheiro):** o veredicto estava certo e mesmo
assim a evidência estava errada. A lição original é *não escrever o número antes
de correr*. Esta é mais funda e é de **assinatura**: delegar o número a um
formatador que não existe não é antecipar um número, é **inventar uma
capacidade**. E o `gcc` diz isso em tempo de compilação — se o aviso fosse lido.

**How to apply (acrescento às quatro regras):**

5. **Um mecanismo de teste que recebe texto tem assinatura.** Antes de meter
   `%d` num `ok()`, ler a assinatura. Texto com especificador de formato num
   sítio que não os expande é um `%d` que vai para o disco, e o gume não o alcança
   porque **o gume olha para a condição, não para a string**.
6. **Um teste que devolve verde não valida o seu próprio relatório.** Fiz o
   controlo negativo — perturbei uma referência e obtive `exit 1` com
   exactamente 1 falha. Isso valida o **canal do veredicto**. Não valida o
   **canal da evidência**: o texto já estava partido na execução que passou, e
   nenhuma perturbação da condição o ia apanhar. São dois canais, e é preciso
   dizer qual se validou.
7. **Aviso de compilação dentro de uma avalanche é um aviso perdido.** Se o build
   emite mais de N linhas, a leitura por blocos deixa de ser leitura. Vale a pena
   contar os avisos e, quando a avalanche tapa um `-Wformat`, ir DIRECTAMENTE às
   de formato. E `2>&1` num `gcc` que compila bem devolve `$?` verdadeiro — o
   `NativeCommandError` é do invólucro do PowerShell, não do compilador.

Mesma família de [[feedback-assercoes-vazias]]: aqui o que passava sem poder
falhar era **a frase**, não a condição.
