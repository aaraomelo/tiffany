---
name: feedback-escrever-numa-notacao-ler-noutra
description: "O eval.txt escreve «A + AB = A» por justaposição; o parser exigia `*`. A casa documentava uma notação e lia outra, e a fala morria calada."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T04:23:55.741Z
---

O `eval.txt` escreve o produto booleano por **justaposição** e o complemento **posfixo**
— «A + AB = A», «F = AB + AB'». Eu implementei o módulo inteiro a partir desse ficheiro,
citei essas identidades nos comentários do `booleana.h`… e o parser exigia `*` explícito.
«simplifica a + ab» respondia *não sei*, calado.

**Why:** a notação da fonte não é decoração — é a **interface**. Quando o documento que
manda escreve `AB` e o leitor exige `A*B`, o sistema documenta uma coisa e aceita outra;
e como a recusa é silenciosa (cai para «não sei»), nenhuma asserção acusa. Foi preciso
eu **sondar à mão** uma fala do próprio ficheiro para dar por isso.

**How to apply:** ao implementar a partir de um documento, extrair as **expressões
literais** que ele escreve e passá-las pelo leitor como caso de teste — não as minhas
transcrições delas. A minha transcrição já é a notação que eu sei ler; é por isso que
passa. Ver [[feedback-a-referencia-escrita-a-mao]]: o mesmo mecanismo, na entrada.

E o gume ficou no **espaço**: sem espaço multiplica-se (`ab`, `xy'`, `a(b+c)`), com
espaço o operador tem de vir escrito — senão o `o` de «a ou b» virava variável e o OU
virava produto. Uma regra permissiva de mais teria trocado a resposta em vez de a
recusar, que é pior. Ver [[feedback-assercoes-vazias]].
