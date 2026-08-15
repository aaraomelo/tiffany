#!/bin/bash
# bateria.sh — roda os medidores citados nos três documentos (teoria, catalogo, enredo), e diz o que cada um devolveu.
#
# A regra do projeto é "resíduo 0 ou falha", e por isso a bateria distingue três coisas:
#   VERDE     o medidor fechou (exit 0)
#   NEGATIVO  o medidor devolveu 1 POR PROJETO — é um teorema negativo, documentado no paper
#             (tatoeba/ancora.c e tatoeba/homogeneo.c: provam que NÃO existe atribuição que feche)
#   FALHA     qualquer outra coisa: não compilou, estourou o tempo, ou quebrou de verdade
#
# --- A BATERIA USA A PRÓPRIA TEORIA -----------------------------------------------------
#
# ATESTADA A SEMENTE, O RESTO É CONSEQUÊNCIA DA DINÂMICA. O medidor é determinista: a mesma
# semente — fonte mais argumentos — dá o mesmo resíduo, sempre. Logo rodar de novo uma semente
# já atestada não acrescenta verdade nenhuma: re-deriva o que a semente já fixou. Seguir o
# rastro é idiotice; a matemática fala por si. O atestado é guardado em tools/atestados.txt e
# NUNCA se apaga — apagar atestado é destruir fato, não "refazer o teste".
#
# (Foi um flag --tudo, que truncava a tabela, que destruiu os atestados do dia 30/07/2026. Ele
#  não existe mais. Para reatestar UM medidor há --reatesta <nome>, e o motivo é de quem chama.)
#
# A teoria do projeto diz por que a decomposição é legítima, e diz exatamente:
#
#   A bateria é uma SOMA DIRETA de componentes independentes — um medidor não fala com outro,
#   como as casas de R^i ⊕ R^j ⊕ R^k. E trio.c §S4 mediu que o caractere de uma soma direta é
#   o PRODUTO dos caracteres das casas: a transformada FATORA. Logo o veredito do todo
#   decompõe-se exatamente nas partes, e recalcular uma não obriga a recalcular as outras.
#   Não é cache com nome bonito — é a licença formal para reaproveitar, e ela é exata.
#
#   E o critério de "mudou" é a ASSINATURA DO CONTEÚDO, não o mtime. assinatura.c mediu que a
#   contagem ASSINA sem perder, e semente.c que assinatura + semente reconstrói a obra exata.
#   O mtime é a régua errada nos dois sentidos: `touch` muda-o sem mudar a obra, e reescrever
#   um arquivo igual não o muda tendo mudado o caminho. A assinatura não se engana em nenhum.
#
#   O SELO é a TRANSFORMADA UNIVERSAL da membrana, e a membrana é o vetor das assinaturas —
#   uma entrada por medidor. transformada.c mede o que autoriza usá-la assim:
#     §U3  ela é UNITÁRIA: ‖Fx‖² = n‖x‖². Nenhuma medida vaza entre os dois lados, e por
#          isso NÃO É PRECISO CALCULAR A TRANSFORMADA para saber a norma dela — Parseval dá.
#     §U4  logo x = 0 ⟺ Fx = 0: o selo é zero exatamente quando tudo está verde, e é imune
#          a cancelamento porque é soma de quadrados.
#     §U5  e o Dirac LOCALIZA: um erro numa casa espalha-se por todo o selo (por isso o selo
#          o vê), e a volta concentra o espalhado naquela casa (por isso se sabe qual abrir).
#
#   Onde o selo é hash e onde é prova, dito sem enfeite: a norma é uma soma de quadrados de
#   assinaturas de 64 bits — detecta mudança com confiança de hash, não com prova. Quem decide
#   o que roda é a comparação assinatura a assinatura, essa sim exata. O selo é o número único
#   para "mudou alguma coisa?", e o ponto de Dirac é o que diz onde.
#
# Memória: cada medidor roda sob ulimit -v 2 GB e timeout, para nunca comer a swap da máquina.
# A saída completa de cada um fica em /tmp/bateria/ — para ver outra fatia LÊ-SE O ARQUIVO,
# nunca se roda outra vez.
#
#   ./tools/bateria.sh                 abre só a semente que ainda não tem atestado
#   ./tools/bateria.sh --refaz         reabre TODAS, sem apagar atestado nenhum (~10 min)
#   ./tools/bateria.sh --selo          imprime o selo e sai, sem abrir nada
#   ./tools/bateria.sh --reatesta X    reatesta o medidor X (compilador novo, máquina nova)

set -u
cd "$(dirname "$0")/.." || exit 1
RAIZ=$PWD
SAIDA=/tmp/bateria; mkdir -p "$SAIDA"
# o atestado vive NO REPO, não em /tmp: é fato sobre a matemática, não sobre esta máquina
TABELA="$RAIZ/tools/atestados.txt"; touch "$TABELA"
SO_SELO=0; REATESTA=""; REFAZ=0
case "${1:-}" in
  --selo)     SO_SELO=1 ;;
  --reatesta) REATESTA="${2:-}"; [ -n "$REATESTA" ] || { echo "uso: --reatesta <medidor>"; exit 1; } ;;
  # A corrida completa, SEM o defeito que a fez ser apagada. O antigo --tudo truncava a tabela
  # de atestados antes de correr — e uma morte a meio deixava o repositório sem os factos que
  # já tinha. Este NUNCA limpa: reabre tudo e vai SUBSTITUINDO linha a linha, de modo que uma
  # interrupção só perde o que ainda não correu. Refazer o teste nunca deve destruir registo.
  --refaz)    REFAZ=1 ;;
esac

# a lista sai dos próprios papers: nada de lista mantida à mão
LISTA=$(mktemp)
# As TRÊS extensões na mesma alternativa, e nas DUAS linhas. O segundo grep — o dos nomes com
# \_ escapado no LaTeX — parava em .c, e por isso um medidor que fosse ao mesmo tempo composto
# e não-C ficava invisível: aparecia como "não citado" estando citado. Só se vê a comparar as
# duas listas, porque o total não desce quando alguém nunca chegou a entrar.
# E OS PAPERS ENTRAM NA VARREDURA. Um medidor citado só num paper (papers/medida.tex foi o
# primeiro) ficava fora da lista e desaparecia em silêncio — o total não desce quando alguém
# nunca chegou a entrar, que é o defeito que esta bateria existe para não ter.
# E conecthus/ é a mesma frase outra vez: o fundamento.tex cita medidores, e um paper que
# cita sem entrar na varredura é um \medido que a bateria não vê.
{ grep -ohE '(tests|banco)/[a-z_0-9]+\.(c|py|js)' teoria.tex catalogo.tex enredo.tex papers/*.tex conecthus/*.tex
  grep -ohE '(tests|banco)/[a-z0-9]+(\\_[a-z0-9]+)+\.(c|py|js)' teoria.tex catalogo.tex enredo.tex papers/*.tex conecthus/*.tex | sed 's/\\_/_/g'
} 2>/dev/null | sort -u > "$LISTA"

# um .pgm de teste para os medidores que leem imagem (linear, venom)
printf 'P5\n32 32\n255\n' > /tmp/bat.pgm
python3 -c "open('/tmp/bat.pgm','ab').write(bytes(((x*7+y*13)%256) for y in range(32) for x in range(32)))" 2>/dev/null

args() { case "$1" in
  neuronio|neuronio_analog) echo "../teoria.tex" ;;
  banco|sql)                echo "teste" ;;
  fala)                     echo "-teste" ;;
  linear|venom)             echo "/tmp/bat.pgm" ;;
  ancora)                   echo "pares.tsv 20000" ;;
  homogeneo|embedding)      echo "pares.tsv" ;;
  regua|centro|bairro)      echo "pares.tsv" ;;
  operador)                 echo "pares.tsv 6 0 0 1" ;;
  *)                        echo "" ;;
esac }

# A LISTA À MÃO ACABOU, e ela era o problema que o Aarão apontou: "qual o problema desses dois
# medidores que não falharam nem passaram?".
#
# `ancora` e `homogeneo` provam teoremas NEGATIVOS — que a soma-de-palavras não é a tradução. Mas
# afirmavam-no ao contrário: a asserção dizia que a rotação FECHA, o resultado era que não fecha, e
# ela falhava sempre. Esta lista traduzia essa falha em "NEGATIVO, teorema por projeto" e calava.
#
# E o preço mediu-se: com o corpus truncado a TRÊS PARES o `ancora` dava exatamente o mesmo veredito
# que com 196 415. Uma asserção que nunca passa é tão vazia quanto uma que nunca falha.
#
# Os dois passaram a dizer o negativo POSITIVAMENTE — "a taxa fica ABAIXO de 5%" e "o corpus tem
# pelo menos N pares" — logo passam, e podem falhar. A lista deixou de ter para que servir, e
# nenhuma outra ocupa o lugar dela: um medidor com direito a falhar não é medido.
negativo_esperado() { return 1; }

# a assinatura de um medidor: o conteúdo do fonte e os argumentos com que corre.
# Nada de mtime — a régua é a obra, não o relógio.
# O .js entra pela MESMA porta que o .c e o .py: uma linguagem é uma realização, e o que a
# bateria conta é o predicado — resíduo 0 —, não o substrato em que corre.
assinatura() { { cat "$1" 2>/dev/null || cat "${1%.c}.py" 2>/dev/null || cat "${1%.c}.js"; printf '%s' "$2"; } | sha256sum | cut -c1-16; }

# --- o selo: ‖Fx‖² = n‖x‖² por Parseval, sobre a membrana das assinaturas ---------------
# Soma de QUADRADOS, não XOR: quadrado não cancela, e é isso que impede duas mudanças de se
# anularem no selo. O n multiplica porque a transformada não é normalizada aqui.
selo() {
  local acc=0 a v n=0
  while read -r f; do
    [ -f "$RAIZ/$f" ] || continue
    a=$(assinatura "$RAIZ/$f" "$(args "$(basename "$f" .c)")")
    v=$(( 0x${a:0:8} ))          # 32 bits, para o quadrado caber em 64 sem estourar
    acc=$(( (acc + v * v) % 0x7fffffffffffffff ))
    n=$(( n + 1 ))
  done < "$LISTA"
  printf '%016x' $(( (acc * n) % 0x7fffffffffffffff ))
}

if [ "$SO_SELO" -eq 1 ]; then
  printf 'selo da bateria: %s   (%d medidores)\n' "$(selo)" "$(wc -l < "$LISTA")"
  rm -f "$LISTA"; exit 0
fi

verde=0; negativo=0; falha=0; total=0; rodados=0; reusados=0; uni_ok=0; uni_ma=0; uni_neg=0; grosso=0
printf '%-26s %-9s %s\n' "MEDIDOR" "SAÍDA" "VEREDITO"
printf '%s\n' "-------------------------------------------------------------------------"

for f in $(cat "$LISTA"); do
  total=$((total+1))
  dir=$(dirname "$f"); base=$(basename "$f" .c); base=${base%.py}; base=${base%.js}
  cd "$RAIZ/$dir" || continue
  bin="$SAIDA/bat_$base"; out="$SAIDA/$base.txt"
  ass=$(assinatura "$base.c" "$(args "$base")")

  # semente atestada? então não se abre — o resíduo é consequência, não descoberta.
  guardado=$(grep -m1 "^$base $ass " "$TABELA" 2>/dev/null)
  [ "$base" = "$REATESTA" ] && guardado=""
  [ "$REFAZ" -eq 1 ] && guardado=""
  if [ -n "$guardado" ]; then
    r=$(printf '%s' "$guardado" | cut -d' ' -f3)
    reusados=$((reusados+1))
    if [ "$r" -eq 0 ]; then verde=$((verde+1))
    elif [ "$r" -eq 1 ] && negativo_esperado "$base"; then negativo=$((negativo+1))
    else printf '%-26s %-9s %s\n' "$f" "FALHA" "exit $r, atestado"; falha=$((falha+1)); fi
    continue
  fi

  # medidor em .py roda com python3 — não compila, e a assinatura é do próprio .py
  if [ -f "$base.py" ] && [ ! -f "$base.c" ]; then
    (ulimit -v 2000000; timeout 300 python3 "$base.py" </dev/null > "$out" 2>&1); r=$?
    printf '%s' "$r" > "$out.exit"
    grep -v "^$base " "$TABELA" > "$TABELA.novo" 2>/dev/null; mv "$TABELA.novo" "$TABELA"
    printf '%s %s %d\n' "$base" "$ass" "$r" >> "$TABELA"
    LC_ALL=C sort -o "$TABELA" "$TABELA"
    rodados=$((rodados+1))
    cert=$(grep -oE 'certificadas *: *[0-9]+/[0-9]+' "$out" 2>/dev/null | tail -1)
    # AS UNIDADES CONTAM-SE AQUI TAMBÉM. Estes dois ramos somavam UMA unidade grossa e nunca
    # liam os `#UNIT` — um medidor de nove unidades entrava no total como uma. O verde/vermelho
    # estava certo (é o exit), mas a soma de unidades mentia por omissão em TODO o .py e .js, e
    # calada: nenhuma asserção o apanhava, porque o número que descia não era o de ninguém.
    u_ok=$(grep -ac '^#UNIT ok' "$out" 2>/dev/null); u_ok=${u_ok:-0}
    u_ma=$(grep -ac '^#UNIT falha' "$out" 2>/dev/null); u_ma=${u_ma:-0}
    if [ "$u_ok" -eq 0 ] && [ "$u_ma" -eq 0 ]; then
      if [ "$r" -eq 0 ]; then u_ok=1; else u_ma=1; fi
      grosso=$((grosso + 1))
    fi
    uni_ok=$((uni_ok + u_ok)); uni_ma=$((uni_ma + u_ma))
    [ "$u_ok$u_ma" != "00" ] && cert="$u_ok unidade(s), $u_ma falha(s) — ${cert:-ok}"
    # EXIT E UNIDADES TÊM DE CONCORDAR. O cards saiu 0 com duas #UNIT falha — um
    # `long falhas` local no main sombreava o contador do unidade.h e o verde era
    # falso (14/08, e havia mais 16 com o mesmo shadow e 3 com `return 0` fixo).
    # A rede fecha a CLASSE: verde com unidade vermelha não é verde, é FALHA — e
    # nenhum medidor futuro precisa de ser confiado, porque os dois caminhos são
    # comparados aqui.
    if [ "$r" -eq 0 ] && [ "$u_ma" -gt 0 ]; then
      r=9
      # a tabela ja' levou o exit 0 — corrige-se, senao o REUSO ressuscitava o verde
      # (a licao do medidor que nunca mediu: a atestacao guarda o resultado FINAL)
      grep -v "^$base " "$TABELA" > "$TABELA.novo" 2>/dev/null; mv "$TABELA.novo" "$TABELA"
      printf '%s %s 9\n' "$base" "$ass" >> "$TABELA"
      LC_ALL=C sort -o "$TABELA" "$TABELA"
    fi
    if [ "$r" -eq 0 ]; then printf '%-26s %-9s %s\n' "$f" "VERDE" "${cert:-ok}"; verde=$((verde+1))
    elif [ "$r" -eq 9 ]; then printf '%-26s %-9s %s\n' "$f" "FALHA" "VERDE FALSO: exit 0 com $u_ma unidade(s) vermelha(s)"; falha=$((falha+1))
    else printf '%-26s %-9s %s\n' "$f" "FALHA" "exit $r"; falha=$((falha+1)); fi
    continue
  fi

  # medidor em .js roda com node — a mesma porta do .py: não compila, e a assinatura é do .js
  # O V8 RESERVA espaço VIRTUAL enorme por memória wasm (guard regions de ~10 GB
  # por instância, sem tocar na residente): ulimit -v de 8 GB matava qualquer
  # medidor wasm com "Out of memory" falso. A proteção que se quer é sobre a
  # RESIDENTE; para o virtual, 64 GB deixam o wasm instanciar sem abrir a swap.
  if [ -f "$base.js" ] && [ ! -f "$base.c" ]; then
    (ulimit -v 200000000; timeout 600 node "$base.js" </dev/null > "$out" 2>&1); r=$?
    printf '%s' "$r" > "$out.exit"
    grep -v "^$base " "$TABELA" > "$TABELA.novo" 2>/dev/null; mv "$TABELA.novo" "$TABELA"
    printf '%s %s %d\n' "$base" "$ass" "$r" >> "$TABELA"
    LC_ALL=C sort -o "$TABELA" "$TABELA"
    rodados=$((rodados+1))
    cert=$(grep -oE 'certificadas *: *[0-9]+/[0-9]+' "$out" 2>/dev/null | tail -1)
    # AS UNIDADES CONTAM-SE AQUI TAMBÉM. Estes dois ramos somavam UMA unidade grossa e nunca
    # liam os `#UNIT` — um medidor de nove unidades entrava no total como uma. O verde/vermelho
    # estava certo (é o exit), mas a soma de unidades mentia por omissão em TODO o .py e .js, e
    # calada: nenhuma asserção o apanhava, porque o número que descia não era o de ninguém.
    u_ok=$(grep -ac '^#UNIT ok' "$out" 2>/dev/null); u_ok=${u_ok:-0}
    u_ma=$(grep -ac '^#UNIT falha' "$out" 2>/dev/null); u_ma=${u_ma:-0}
    if [ "$u_ok" -eq 0 ] && [ "$u_ma" -eq 0 ]; then
      if [ "$r" -eq 0 ]; then u_ok=1; else u_ma=1; fi
      grosso=$((grosso + 1))
    fi
    uni_ok=$((uni_ok + u_ok)); uni_ma=$((uni_ma + u_ma))
    [ "$u_ok$u_ma" != "00" ] && cert="$u_ok unidade(s), $u_ma falha(s) — ${cert:-ok}"
    # EXIT E UNIDADES TÊM DE CONCORDAR. O cards saiu 0 com duas #UNIT falha — um
    # `long falhas` local no main sombreava o contador do unidade.h e o verde era
    # falso (14/08, e havia mais 16 com o mesmo shadow e 3 com `return 0` fixo).
    # A rede fecha a CLASSE: verde com unidade vermelha não é verde, é FALHA — e
    # nenhum medidor futuro precisa de ser confiado, porque os dois caminhos são
    # comparados aqui.
    if [ "$r" -eq 0 ] && [ "$u_ma" -gt 0 ]; then
      r=9
      # a tabela ja' levou o exit 0 — corrige-se, senao o REUSO ressuscitava o verde
      # (a licao do medidor que nunca mediu: a atestacao guarda o resultado FINAL)
      grep -v "^$base " "$TABELA" > "$TABELA.novo" 2>/dev/null; mv "$TABELA.novo" "$TABELA"
      printf '%s %s 9\n' "$base" "$ass" >> "$TABELA"
      LC_ALL=C sort -o "$TABELA" "$TABELA"
    fi
    if [ "$r" -eq 0 ]; then printf '%-26s %-9s %s\n' "$f" "VERDE" "${cert:-ok}"; verde=$((verde+1))
    elif [ "$r" -eq 9 ]; then printf '%-26s %-9s %s\n' "$f" "FALHA" "VERDE FALSO: exit 0 com $u_ma unidade(s) vermelha(s)"; falha=$((falha+1))
    else printf '%-26s %-9s %s\n' "$f" "FALHA" "exit $r"; falha=$((falha+1)); fi
    continue
  fi
  if ! cc -O2 -std=c99 -I. -I../tools -I../lib -I../tests "$base.c" -lm -o "$bin" 2>/dev/null; then
    printf '%-26s %-9s %s\n' "$f" "—" "NÃO COMPILOU"; falha=$((falha+1)); continue
  fi
  rodados=$((rodados+1))
  if [ "$base" = dente ]; then          # dente roda em duas etapas, com sort externo no meio
    (ulimit -v 2000000; timeout 200 "$bin" emite pares.tsv > "$SAIDA/dente_sig.txt" 2>/dev/null) &&
      LC_ALL=C sort -S 64M "$SAIDA/dente_sig.txt" | (ulimit -v 2000000; timeout 120 "$bin" agrupa > "$out" 2>&1)
    r=$?
  else
    (ulimit -v 2000000; timeout 560 "$bin" $(args "$base") </dev/null > "$out" 2>&1); r=$?
  fi
  # atesta a semente. Só a linha DESTE medidor é substituída — nunca se limpa a tabela.
  grep -v "^$base " "$TABELA" > "$TABELA.novo" 2>/dev/null; mv "$TABELA.novo" "$TABELA"
  printf '%s %s %d\n' "$base" "$ass" "$r" >> "$TABELA"
  LC_ALL=C sort -o "$TABELA" "$TABELA"

  ver=$(grep -ohE 'RESIDUO 0|RESÍDUO 0|resíduo 0|resíduo total = 0|residuo=0|resíduo=0|viol=0|O DENTE|FALHOU|FALHA' "$out" 2>/dev/null | tail -1)
  # grep -c IMPRIME 0 e DEVOLVE 1 quando não acha: o "|| echo 0" produzia "0\n0" e a
  # aritmética morria — e o laço inteiro morria com ela, deixando o relatório VERDE sobre
  # dois medidores de oitenta e três. Nada de || aqui.
  u_ok=$(grep -ac '^#UNIT ok' "$out" 2>/dev/null); u_ok=${u_ok:-0}
  u_ma=$(grep -ac '^#UNIT falha' "$out" 2>/dev/null); u_ma=${u_ma:-0}
  # O MÍNIMO HONESTO. Um punhado de medidores usa idioma próprio e não emite unidade fina.
  # Em vez de os deixar contando ZERO — o que faria a soma de unidades mentir por omissão —
  # cada um conta UMA: o próprio veredito de saída. É grosso, e é dito que é grosso; o que
  # não se pode é somar 0 e parecer que não havia nada a contar.
  if [ "$u_ok" -eq 0 ] && [ "$u_ma" -eq 0 ]; then
    if [ "$r" -eq 0 ] || { [ "$r" -eq 1 ] && negativo_esperado "$base"; }; then u_ok=1; else u_ma=1; fi
    grosso=$((grosso + 1))
  fi
  # Um teorema negativo por projeto FALHA a sua asserção de propósito — é isso que ele prova.
  # Somar essa falha ao contador geral fazia o relatório dizer "0 falhas" e "2 falharam" na
  # mesma saída, o que é incoerência e não informação. Vai para o seu próprio balde.
  if negativo_esperado "$base"; then uni_ok=$((uni_ok + u_ok)); uni_neg=$((uni_neg + u_ma))
  else uni_ok=$((uni_ok + u_ok)); uni_ma=$((uni_ma + u_ma)); fi
  # ${ver:-ok} e nao ${ver}: o ramo .py/.js ja' escrevia ${cert:-ok}, e este nao. Um medidor
  # que nao imprima nenhum dos tokens do veredicto ("RESIDUO 0", "FALHOU", ...) saia' com um
  # travessao pendurado — "VERDE  11 unidade(s), 0 falha(s) —". As duas metades do mesmo if,
  # e a que ficou para tras foi a que ninguem exercitou ate' agora.
  [ "$u_ok$u_ma" != "00" ] && ver="$u_ok unidade(s), $u_ma falha(s) — ${ver:-ok}"
  # a mesma rede do ramo .py/.js: exit 0 com unidade vermelha é FALHA, não verde
  if ! negativo_esperado "$base" && [ "$r" -eq 0 ] && [ "$u_ma" -gt 0 ]; then
    r=9
    grep -v "^$base " "$TABELA" > "$TABELA.novo" 2>/dev/null; mv "$TABELA.novo" "$TABELA"
    printf '%s %s 9\n' "$base" "$ass" >> "$TABELA"
    LC_ALL=C sort -o "$TABELA" "$TABELA"
  fi
  if [ "$r" -eq 9 ]; then
    printf '%-26s %-9s %s\n' "$f" "FALHA" "VERDE FALSO: exit 0 com $u_ma unidade(s) vermelha(s)"; falha=$((falha+1))
  elif [ "$r" -eq 0 ]; then
    printf '%-26s %-9s %s\n' "$f" "VERDE" "${ver:-ok}"; verde=$((verde+1))
  elif [ "$r" -eq 1 ] && negativo_esperado "$base"; then
    printf '%-26s %-9s %s\n' "$f" "NEGATIVO" "teorema negativo por projeto — ${ver:-ver paper}"; negativo=$((negativo+1))
  else
    printf '%-26s %-9s %s\n' "$f" "FALHA" "exit $r — ${ver:-sem veredito}"; falha=$((falha+1))
  fi
done

cd "$RAIZ" || exit 1

# --- deriva: medidor que existe no disco e nenhum paper cita NUNCA roda aqui ---
# Sem esta conferência um medidor apodrece em silêncio: a lista sai dos PAPERS, então o que
# não é citado não é testado — e a contagem parece completa sem estar.
cp "$LISTA" /tmp/bat_citados.txt
# CORRIGIDO: os medidores vivem em tests/ e banco/, nao em tools/. Enquanto esta linha
# olhava so' para tools/, a lista de existentes saia vazia, os 282 citados apareciam
# todos como REFERENCIA QUEBRADA — e a conferencia inversa (medidor no disco que nenhum
# paper cita) nunca podia disparar, que e' exatamente o que ela existe para apanhar.
ls tests/*.c banco/*.c tools/*.c tests/morfico.py tests/*.js tatoeba/*.c 2>/dev/null | sort > /tmp/bat_existem.txt
# Quem está declarado como NÃO-MEDIDOR sai só da conta dos NÃO CITADOS — nunca da conta do que
# EXISTE. Eu tinha tirado do "existe", e aí um arquivo declarado E citado aparecia como
# REFERÊNCIA QUEBRADA: o ficheiro está no disco, só não afirma nada. O filtro estava no lado
# errado da comparação.
: > /tmp/bat_declarados.txt
[ -f tools/NAO-MEDIDORES.txt ] && grep -oE '^(tools|tatoeba)/[a-z_0-9]+\.c' tools/NAO-MEDIDORES.txt | sort -u > /tmp/bat_declarados.txt
quebradas=$(comm -23 /tmp/bat_citados.txt /tmp/bat_existem.txt | wc -l)
comm -13 /tmp/bat_citados.txt /tmp/bat_existem.txt | comm -23 - /tmp/bat_declarados.txt > /tmp/bat_nc.txt
naocitados=$(wc -l < /tmp/bat_nc.txt)
if [ "$quebradas" -gt 0 ]; then
  printf 'REFERENCIA QUEBRADA: %d citado(s) nos papers que nao existem no disco:\n' "$quebradas"
  comm -23 /tmp/bat_citados.txt /tmp/bat_existem.txt | sed 's/^/    /'
fi
if [ "$naocitados" -gt 0 ]; then
  printf 'nao citados (existem, nenhum paper cita, logo NAO sao testados): %d\n' "$naocitados"
  # a LISTA tem de ser a mesma que o CONTADOR — imprimia a não-filtrada e contava a
  # filtrada, e o relatório dizia "1" com sete linhas por baixo. Um relatório que se
  # contradiz é pior que um que cala: parece informação e não é.
  sed 's/^/    /' /tmp/bat_nc.txt
fi

printf '%s\n' "-------------------------------------------------------------------------"
# ── ATESTACOES ORFAS: linhas de medidores que ja' nao existem ────────────────────
# Uma atestacao orfa nao mascara nada por si — nao ha' ficheiro para correr. Mas foi a
# procurar as que guardavam exit != 0 que se descobriu o transfusao_real, que NUNCA
# MEDIU e estava atestado a 2 desde sempre. A tabela tem de dizer quando esta' a
# carregar historia de coisas que ja' nao ha'.
# ── NÃO MEDIU ≠ FALHOU ───────────────────────────────────────────────────────────
# Três medidores (transfusao_real, dualcifra, protocolo) não medem sem dados do doador,
# e dizem-no na saída: "NAO MEDIU". Durante meses estiveram atestados com exit 2 e
# contados como VERDES — a bateria dizia 288/288 com três a nunca terem medido.
#
# Contá-los como falha é honesto (não medir não é passar), mas some no total. Aqui
# separam-se: o número diz quantos, e a linha diz que basta correr colhe_tudo.sh.
mudos=0
for _f in "$SAIDA"/*.txt; do
  [ -f "$_f" ] || continue
  # so' conta quem AINDA e' da arvore: as saidas velhas de medidores movidos
  # (o doador foi para integration/ em 14/08) ficavam em /tmp e o aviso
  # «faltam dados do doador» nunca morria — contava fantasmas, nao medidores
  _b=$(basename "$_f" .txt)
  [ -f "$RAIZ/tests/$_b.c" ] || [ -f "$RAIZ/banco/$_b.c" ] || [ -f "$RAIZ/tatoeba/$_b.c" ] ||   [ -f "$RAIZ/tests/$_b.py" ] || [ -f "$RAIZ/tests/$_b.js" ] || continue
  grep -q "NAO MEDIU\|NÃO MEDIU" "$_f" 2>/dev/null && mudos=$((mudos+1))
done
if [ "$mudos" -gt 0 ]; then
  printf 'MUDOS: %d medidor(es) disseram NAO MEDIU — faltam dados do doador.\n' "$mudos"
  printf '       o sistema e AUTO-CONTIDO: nao ha nada de fora para acordar\n'
fi

orfas=0
while read -r _n _a _r; do
  [ -z "$_n" ] && continue
  [ -f "$RAIZ/tests/$_n.c" ] || [ -f "$RAIZ/banco/$_n.c" ] || \
  [ -f "$RAIZ/tatoeba/$_n.c" ] || [ -f "$RAIZ/tests/$_n.py" ] || [ -f "$RAIZ/tests/$_n.js" ] || orfas=$((orfas+1))
done < "$TABELA"
[ "$orfas" -gt 0 ] && printf 'ATENCAO: %d atestacoes ORFAS na tabela — medidores que ja nao existem\n' "$orfas"

printf 'total %d : %d verdes, %d negativos por projeto, %d falhas\n' "$total" "$verde" "$negativo" "$falha"

# A GUARDA CONTRA O VERDE FALSO.
# Uma bateria que percorre 2 dos 83 e diz "0 falhas" está a mentir, e foi exatamente isso que
# aconteceu: um erro de aritmética matou o laço na primeira linha e o relatório saiu verde
# sobre nada. Contar quantos DEVIAM ser percorridos e comparar é barato, e é o que impede.
esperados=$(wc -l < "$LISTA")
if [ "$total" -ne "$esperados" ]; then
  printf 'VERDE FALSO: a lista tem %d medidores e o laço percorreu %d.\n' "$esperados" "$total"
  printf 'O relatório acima NÃO vale. Alguma coisa interrompeu a varredura.\n'
  falha=$((falha + 1))
fi
printf 'selo %s : %d sementes abertas agora, %d já atestadas (nada a re-derivar)\n' "$(selo)" "$rodados" "$reusados"
printf 'unidades: %d passaram, %d falharam, %d negativas por projeto (nas abertas agora)\n' "$uni_ok" "$uni_ma" "$uni_neg"
[ "$grosso" -gt 0 ] && printf '  (%d desses medidores contam 1 unidade GROSSA — o exit — por ainda\n   usarem idioma próprio. Não é fineza; é o mínimo para a soma não mentir.)\n' "$grosso"
printf 'saída de cada medidor em %s/ — para ver outra fatia LEIA O ARQUIVO, não rode de novo.\n' "$SAIDA"
rm -f "$LISTA"
[ "$falha" -eq 0 ] && [ "$quebradas" -eq 0 ] || exit 1
