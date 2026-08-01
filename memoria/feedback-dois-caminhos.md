---
name: feedback-dois-caminhos
description: "Comparar dois caminhos que TÊM de concordar apanha o que nenhuma asserção sobre um caminho só apanha — e ler o número que mede, não o que está à mão"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T13:44:35.649Z
---

# Dois caminhos que têm de concordar

Em 01/08/2026 os dois piores defeitos da sessão foram encontrados **não por asserções**, mas por
uma linha que compara os dois caminhos de resolver a mesma expressão (dobrar por dentro vs.
distribuir) e comenta: *"não devia diferir; há defeito aqui"*.

Nas duas vezes o defeito era o mesmo em camadas diferentes: quando a célula ganhou o denominador,
e depois quando ganhou a parte imaginária, as cópias da reescrita apagavam-nas. **Todas as
asserções passavam verdes**, porque os seus casos eram todos de inteiros.

**Porquê:** uma asserção mede um caminho contra um valor que eu escrevi. Se eu errei a pensar, ela
confirma o meu erro. Dois caminhos independentes que *têm* de fechar no mesmo não precisam que eu
saiba a resposta — eles denunciam-se um ao outro.

**Como aplicar:**

1. Onde houver duas maneiras de chegar ao mesmo (uma lei, um dual, uma identidade), **medir as
   duas e comparar**, em vez de medir uma contra um valor esperado.
2. Os casos do teste têm de incluir o **caso difícil**, não só o fácil. O teste dos dois caminhos
   passava verde porque todos os seus exemplos eram inteiros — e o difícil era só ter um `/2`.
3. **Copiar structs inteiras, não campo a campo.** Copiar campo a campo obriga a lembrar de todos
   os campos, e uma componente nova perde-se em silêncio.

## A terceira vez — e era do outro lado

Em 01/08/2026, ao introduzir o `i*` (a unidade dual), o mesmo defeito voltou pela **terceira**
vez: `1 / i*` respondia *"dá i"*. Certo por dentro, errado por fora.

A correção da segunda vez (`ct_cel`, copiar a struct) tinha fechado só o lado da **escrita**. O
lado da **leitura** continuava a extrair campo a campo — `ct_valorc(fd, n, &p, &q, &ip, &iq)` —
e o campo novo (`sig`) não estava na lista. Corrigido com `ct_valorcel`, que devolve a `Cel`
inteira.

**Como aplicar:** quando um tipo ganha uma componente, procurar **os dois lados** — quem escreve
e quem lê. Uma função cuja assinatura enumera os campos é o sítio onde o próximo campo se vai
perder; se ela existe, é o defeito à espera. E medir a **resposta**, não a fita: as três vezes
(`7/2` → "7", `raiz -4` → "0", `1/i*` → "i") passaram verdes porque o medidor lia a
representação intermédia e não o valor final.

# E ler o número que mede

O `sql.c` — 87 asserções, o medidor maior do projeto — não compilava com a linha da bateria e
estava **fora da medida há três corridas**. Passou porque eu lia `unidades: 7 passaram, 0
falharam` e dava por verde. Sete. Com ele dentro são cem, e o `1 falhas` do total estava à minha
frente em todos os commits.

**Antes de dar por verde: ver o TOTAL e a coluna de compilação, não só a linha das unidades
abertas.** Um medidor que não compila não falha — desaparece. Ver [[project-checkpoint-2026-08-01]].

# E a margem larga esconde

Em 01/08 (noite) uma asserção media "iω a menos do erro O(h²)" com tolerância 0,2, e o valor era
2,3526. **Não era 3 com erro: era o símbolo EXATO do operador que eu de facto usei**
(i·sen(ωh)/h, a diferença central). A margem larga teria dado verde e escondido que o objeto
medido não era o que eu dizia.

**Como aplicar:** quando um valor não bate, perguntar primeiro se o que se está a medir é mesmo o
que se pensa — e só depois alargar a margem. Uma tolerância generosa transforma um achado em ruído.
