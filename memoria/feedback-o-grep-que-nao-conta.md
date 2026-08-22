---
name: feedback-o-grep-que-nao-conta
description: "`grep` é uma FUNÇÃO de shell nesta máquina e engole a saída de `-c`: declarei «0 erros» num log com 5."
metadata:
  type: feedback
---

Compilei o `aranha.tex`, corri `grep -c "^! " aranha.log`, **não saiu nada**, e li
o vazio como **zero**. Anunciei ao Aarão «0 erros, 0 referências por resolver».

O log tinha **cinco** `! Extra alignment tab has been changed to \cr` — eu tinha
acrescentado uma quarta coluna ao `array` do Lema da restrição cristalográfica e
deixado o preâmbulo em `{ccl}`. O LaTeX **recupera sozinho** («assume que quis
`\cr`»), o PDF sai com o número de páginas certo, e a tabela fica partida.

`type grep` → **«grep é uma função»**. O perfil desta máquina embrulha o `grep`, e
o embrulho não devolve a contagem do `-c`.

**Why:** um contador que não imprime é indistinguível de um contador que imprime
zero — é o canal de erro a reutilizar um valor do domínio, outra vez, mas agora no
MEU instrumento e não no medido. Todo o resto da sessão passou por ali: cada
`grep -c undefined` que li como «0» não mediu nada.

**How to apply:**
1. **Contar com `| wc -l`**, ou `command grep` para saltar a função. Nunca `-c`.
2. **Um número tem de aparecer.** Se a saída de uma verificação pode ser vazia,
   embrulhá-la em `echo "rótulo: $(… | wc -l)"` para que o zero SEJA impresso.
3. **O `pdflatex` não falha onde o LaTeX recupera.** Páginas certas e `Output
   written` não são atestado: `! ` no log é o único.
4. E o cwd do Bash **persiste**: `cd papers` a partir de `papers/` falha, o `&&`
   corta, e o comando seguinte lê o log ANTIGO. Caminho absoluto, sempre.

Ver [[feedback-saturacao-nao-e-resultado]] (o valor de erro que colide com um
legítimo), [[feedback-assercoes-vazias]] e [[feedback-a-macro-por-definir]].
